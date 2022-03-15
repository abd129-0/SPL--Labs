#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> 
#include <wait.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <fcntl.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define WriteEnd 1
#define ReadEnd 0
#define history_size 10

int debugMode = 0;
char* history[history_size];
int history_index = 0;

void redirectIn(const char* from, int to){
    int fd = open(from, O_RDONLY);
    dup2(fd, to); //redirect
    close(fd);
}
void redirectOut(const char* from, int to){
    int fd;
    if ((fd = open(from, O_CREAT | O_WRONLY, 0777)) < 0)
        fprintf(stderr, "error opening output");
    dup2(fd, to); //redirect
    close(fd);
}

int child(int pipefd[], int toClose, int i, cmdLine* cmd, int debug){
    if (debug) fprintf(stderr, "(parent_process>forking…)\n");
    int childPID = fork();
    if(childPID == 0){
        if(cmd->inputRedirect != NULL) redirectIn(cmd->inputRedirect, STDIN);
        if(cmd->outputRedirect != NULL) redirectOut(cmd->outputRedirect, STDOUT);
        if (debug){
            if(i)
                fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
            else 
                fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
        }
        close(toClose);
        dup(pipefd[i]);
        close(pipefd[i]);
        if (debug) fprintf(stderr, "(child>going to execute cmd: %s)\n", cmd->arguments[0]);
        execvp(cmd->arguments[0], cmd->arguments);  
        perror("execvp");
        _exit(0);
    }
    return childPID;
}

void execute(cmdLine *pCmdLine){
    if(strncmp(pCmdLine->arguments[0], "history", 7) == 0){
        for(int i = 0; (i < history_index) & (i < 10); i++)
            fprintf(stderr, "%s", history[i]);
    } else if(strncmp(pCmdLine->arguments[0], "cd", 2) == 0){
        chdir(pCmdLine->arguments[1]);
    } else if (pCmdLine->next != NULL) {
        int pipefd[2], child1, child2;
        if (pipe(pipefd) == -1){
            perror("pipe\n");
            _exit(0);
        }
        child1 = child(pipefd, STDOUT_FILENO, WriteEnd, pCmdLine, debugMode);
        if(debugMode) fprintf(stderr, "(parent_process>created process with id: %d)\n", child1);
        if(debugMode) fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
        close(pipefd[WriteEnd]);
        child2 = child(pipefd, STDIN_FILENO, ReadEnd, pCmdLine->next, debugMode);
        if(debugMode) fprintf(stderr, "(parent_process>created process with id: %d)\n", child2);
        if(debugMode) fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
        close(pipefd[ReadEnd]);
        if(debugMode) fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
        waitpid(child1, NULL, 0);
        waitpid(child2, NULL, 0);
        if(debugMode) fprintf(stderr, "(parent_process>exiting…)\n");
    } else {
        int childPID = fork();
        if(childPID == 0) {
            if(pCmdLine->inputRedirect != NULL) redirectIn(pCmdLine->inputRedirect, STDIN);
            if(pCmdLine->outputRedirect != NULL) redirectOut(pCmdLine->outputRedirect, STDOUT);
            execvp(pCmdLine->arguments[0], pCmdLine->arguments);
            perror("Error");
            exit(-1);
        }
        if(pCmdLine->blocking == 1)
            waitpid(childPID, NULL, 0);
    }
}

int main(int argc, char** argv){
    char cwd[PATH_MAX];
    char input[2048];
    getcwd(cwd, PATH_MAX);
    cmdLine* cmd;
        for(int i = 1; i < argc; i++)
            if(strcmp(argv[i], "-d") == 0)
                debugMode = 1;
    while(1){
        getcwd(cwd, PATH_MAX);
        fprintf(stderr, "%s: ", cwd);
        fgets(input, 2048, stdin);
        cmd = parseCmdLines(input);
        if(strncmp(cmd->arguments[0], "quit", 4) == 0)
            break;
        if(strncmp(cmd->arguments[0], "!", 1) == 0){
            //!5
            cmdLine* temp = parseCmdLines(history[atoi(cmd->arguments[0]+1)]);
            strcpy(input, history[atoi(cmd->arguments[0]+1)]);
            execute(temp);
        } else execute(cmd);
        if (history_size > 10) {
            free(history[history_index%history_size]);
        }
        history[history_index%history_size] = malloc(strlen(input));
        strcpy(history[history_index%history_size],input); //cycle
        history_index++;
        freeCmdLines(cmd);
    }
    for (int i = 0; (i < history_index) & (i < 10); i++){
        free(history[i]);
    }
    freeCmdLines(cmd);
    return 0;
}