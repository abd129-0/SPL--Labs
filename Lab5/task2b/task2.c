#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> 
#include <wait.h>
#include <linux/limits.h>
#include "LineParser.h"

#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0
/*------------------------------Struct------------------------------*/
typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;
/*--------------------------Help Function--------------------------*/
char* signalAsString(int sig){
    return (sig == TERMINATED) ? "Terminated" :
        (sig == RUNNING) ? "Running" :
        (sig == SUSPENDED) ? "Suspended" :
        "Unknown signal";
}
/*------------------------------Task2b------------------------------*/
void freeProcessList(process* process_list){
    process* temp = process_list;
    while(process_list != NULL){
        temp = process_list;
        process_list = process_list->next;
        freeCmdLines(temp->cmd);
        free(temp);
    }
}

void updateProcessStatus(process* process_list, int pid, int status){
    while(process_list != NULL){
        if(process_list->pid == pid){
            process_list->status = status;
            break;
        }
        process_list = process_list->next;
    }
}

void updateProcessList(process** process_list){
    int status, newStatus;
    pid_t ret;
    process* temp = *process_list;
    while(temp != NULL){
        //returns 0 if status didn't change
        ret = waitpid(temp->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if(ret != 0){
            if(WIFEXITED(status) | WIFSIGNALED(status))
                newStatus = TERMINATED;
            else if(WIFSTOPPED(status))
                newStatus = SUSPENDED;
            else if(WIFCONTINUED(status))
                newStatus = RUNNING;
            updateProcessStatus(*process_list,temp->pid, newStatus);
        }
        temp = temp->next;
    }
}
/*------------------------------Task2a------------------------------*/
void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* temp = malloc(sizeof(process));
    temp->cmd = cmd;
    temp->pid = pid;
    temp->status = RUNNING;
    if(*process_list == NULL){
        temp->next = NULL;
        *process_list = temp;
    } else {
        temp->next = *process_list;
        *process_list = temp;
    }
}

void printProcessList(process** process_list){
    updateProcessList(process_list);
    process* temp = *process_list;
    fprintf(stderr, "PID\tCommand\tSTATUS\n");
    while(temp != NULL){
        fprintf(stderr, "%d\t%s\t%s\n", temp->pid, temp->cmd->arguments[0],signalAsString(temp->status));
        temp = temp->next;
    }
}
/*------------------------------Task1------------------------------*/
int execute(cmdLine *pCmdLine){
    int childPID = fork();
    if(childPID == 0) {   //child
        //exits on success
        execvp(pCmdLine->arguments[0], pCmdLine->arguments); 
        //if reached here, it's an error
        perror("Error");
        freeCmdLines(pCmdLine);
        _exit(-1);
    }
    return childPID;
}

int main(int argc, char** argv){
    char cwd[PATH_MAX];
    char input[2048];
    getcwd(cwd, PATH_MAX);
    cmdLine* cmd;
    process* process_list = NULL;
    while(1){
        fprintf(stderr, "%s: ", cwd);
        fgets(input, 2048, stdin);
        cmd = parseCmdLines(input);
        if(strncmp(cmd->arguments[0], "quit", 4) == 0)
            break;
        if(strncmp(cmd->arguments[0], "cd", 2) == 0){
            if(chdir(cmd->arguments[1]) == 0)
                getcwd(cwd, PATH_MAX);
        } else if(strncmp(cmd->arguments[0], "procs", 5) == 0){
            printProcessList(&process_list);
        }else{
            int pid = execute(cmd);
            addProcess(&process_list, cmd, pid);
            if(cmd->blocking == 1)
                waitpid(pid, NULL, 0);
        }
    }
    freeProcessList(process_list);
    return 0;
}