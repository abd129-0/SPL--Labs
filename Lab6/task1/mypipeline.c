#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> 
#include <wait.h>
#include <linux/limits.h>
#include "LineParser.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define WriteEnd 1
#define ReadEnd 0

int child(int pipefd[], int toClose, int i, char* args[], int debug){
    if (debug) fprintf(stderr, "(parent_process>forking…)\n");
    int childPID = fork();
    if(childPID == 0){
        if (debug){
            if(i)
                fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
            else 
                fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
        }
        close(toClose);
        dup(pipefd[i]);
        close(pipefd[i]);
        if (debug) fprintf(stderr, "(child>going to execute cmd: %s)\n", args[0]);
        execvp(args[0], args);  
        perror("execvp");
        _exit(0);
    }
    return childPID;
}

int main(int argc, char** argv){
    int pipefd[2], debugMode = 0, child1, child2;
    for(int i = 1; i < argc; i++)
        if(strcmp(argv[i], "-d") == 0)
            debugMode = 1;
    if (pipe(pipefd) == -1){
        perror("pipe\n");
        _exit(0);
    }
    char* args[3] = {"ls", "-l", NULL};
    child1 = child(pipefd, STDOUT_FILENO, WriteEnd, args, debugMode);
    if(debugMode) fprintf(stderr, "(parent_process>created process with id: %d)\n", child1);
    if(debugMode) fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
    close(pipefd[WriteEnd]);
    char* args2[4] = {"tail", "-n", "-2", NULL};
    child2 = child(pipefd, STDIN_FILENO, ReadEnd, args2, debugMode);
    if(debugMode) fprintf(stderr, "(parent_process>created process with id: %d)\n", child2);
    if(debugMode) fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
    close(pipefd[ReadEnd]);
    if(debugMode) fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    if(debugMode) fprintf(stderr, "(parent_process>exiting…)\n");
    return 0;
}