
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0


#define STDIN_FAILURE -1
#define STDOUT_FAILURE -1
#define MAX_INPUT_SIZE 2048
#define HISTLEN 20 // part 4


int debugF=0; // zero means debug off - have to be global , otherwise signatures of the functions will need to change.
void execute(cmdLine *pCmdLine);
typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;
process* processList = NULL;
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);// Receive a process list (process_list), a command (cmd), and the process id (pid) of the process running the command. Note that process_list is a pointer to a pointer so that we can insert at the beginning of the list if we wish.
void printProcessList(process** process_list);// print the processes.
void freeProcessList(process* process_list);// free the memory of the process list.
void updateProcessList(process **process_list);// update the process list to remove terminated processes.
void updateProcessStatus(process* process_list, int pid, int status);// update the status of a process in the process list.
void addToHistory(char *cmd,char **history, int *newest, int *oldest, int *historyCount);
void printHistory(int *oldest, int historyCount, char **history);
void func_main_helper(char input[], char *history[], int newest, int oldest, int historyCount);
void free_history(char **history, int historyCount);


int main(int argc,char ** argv) {
    char *history[HISTLEN];
    int newest = 0, oldest = 0;
    int historyCount = 0;

    if ((argc > 1) && (strcmp(argv[1],"-d") == 0)) {
        debugF = 1; //on
    }

    while (1) {
        char input[MAX_INPUT_SIZE];
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current directory: %s\n", cwd);
        } else {
            perror("getcwd did not work!");
            exit(EXIT_FAILURE);
        }

        printf("Enter command: ");
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {//T0a2
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        // Remove the newline character at the end
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, "quit") == 0) {
            printf("Exit shell.\n");
            fprintf(stderr, "free processList\n");
            freeProcessList(processList);
            fprintf(stderr, "free parseCmdLines\n");
            freeCmdLines(parseCmdLines(input));
            fprintf(stderr,"free history\n");
            free_history(history, historyCount);
            break;
        } else{
            func_main_helper(input, history, newest, oldest, historyCount);
        }
        addToHistory(input, history, &newest, &oldest, &historyCount);

    }

    return 0;
}

void func_main_helper(char input[], char *history[], int newest, int oldest, int historyCount){

    if (strncmp(input,"cd ", 3)==0) {
        char *path_cd = input + 3; // moving the pointer to extract the path after "cd "
        if (chdir(path_cd) == -1) {
            //chdir:This command returns zero (0) on success.
            // -1 is returned on an error and errno is set appropriately.
            fprintf(stderr, "cd: %s: Could not found such file or directory \n", path_cd);
        }
    }else if (strcmp(input, "procs") == 0) {
        printProcessList(&processList);
    }else if (strcmp(input, "history") == 0) {
        printHistory(&oldest, historyCount, history);
    }else if (strcmp(input, "!!") == 0) {
        if (historyCount == 0) {
            fprintf(stderr, "No commands in history.\n");
        } else {
            strcpy(input, history[newest-1]);
            if (debugF == 1) {
                fprintf(stderr, "Executing command: %s\n", input);
            }
            func_main_helper(input, history, newest, oldest, historyCount);
        }
    }else if (strncmp(input, "!", 1) == 0) {
        int index = atoi(input + 1);
        if (index > historyCount || index <= 0) {
            fprintf(stderr, "No such command in history.\n");
        } else {
            strcpy(input, history[(oldest + index - 1) % HISTLEN]);
            if (debugF == 1) {
                fprintf(stderr, "Executing command: %s\n", input);
            }
           func_main_helper(input, history, newest, oldest, historyCount);
        }
    }
    else{
        cmdLine *parsedCmd = parseCmdLines(input);// Parse the input
        execute(parsedCmd);// Execute the command
    }
}

void addToHistory(char *cmd, char **history, int *newest, int *oldest, int *historyCount) {
    if (*historyCount >= HISTLEN) {
        free(history[*oldest]);
        *oldest = (*oldest + 1) % HISTLEN;
    } else {
        (*historyCount)++;
    }
    history[*newest] = (char *)malloc(strlen(cmd) + 1);
    strcpy(history[*newest], cmd);
    *newest = (*newest + 1) % HISTLEN;
}

void printHistory(int *oldest, int historyCount, char **history) {
    int i = *oldest, count = 1;
    while (count <= historyCount) {
        printf("%d: %s\n", count, history[i]);
        i = (i + 1) % HISTLEN;
        count++;
    }
}



void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* newProcess = (process*)malloc(sizeof(process));
    if(newProcess == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    } else {
        newProcess->cmd = cmd;
        newProcess->pid = pid;
        newProcess->status = RUNNING;
        newProcess->next = *process_list;
        *process_list = newProcess;
    }
}

void free_history(char **history, int historyCount) {
    int i = 0;
    while (i < historyCount) {
        free(history[i]);
        i++;
    }
}


void printProcessList(process** process_list){
    updateProcessList(process_list);
    printf("PID\t\tCommand\t\tSTATUS\n");
    process* current = *process_list;
    while (current != NULL) {
        const char* statusStr = (current->status == RUNNING) ? "Running" :
                                (current->status == SUSPENDED) ? "Suspended" : "Terminated";

        // Ensure we're accessing the command correctly.
        // cmd->arguments[0] should point to the command name.
        // Using %s to print a string, so it must be null-terminated.
        if (current->cmd && current->cmd->arguments[0]) {
            printf("%d\t\t%s\t\t%s\n", current->pid, current->cmd->arguments[0], statusStr);
        } else {
            // Fallback in case something is wrong with the command.
            printf("%d\t\t[Unknown Command]\t\t%s\n", current->pid, statusStr);
        }

        current = current->next;
    }
}


void freeProcessList(process* process_list){
    process* current = process_list;
    process* next;
    while (current != NULL) {
        next = current->next;
        freeCmdLines(current->cmd);
        free(current);
        current = next;
    }
}

void updateProcessList(process **process_list){
    int begin = 0;
    process* current = *process_list;
    while(current)
    {
        int rID = waitpid(current->pid, &begin, WNOHANG | WUNTRACED | WCONTINUED);
        if(WIFCONTINUED(begin))
        {
            updateProcessStatus(current, current->pid, RUNNING);
            break;
        }
        else if(WIFSTOPPED(begin)){
            updateProcessStatus(current, current->pid, SUSPENDED);
            break;
        }
        else if(rID != 0) {
            updateProcessStatus(current, current->pid, TERMINATED);
        }

        current = current->next;
    }
}

void updateProcessStatus(process* process_list, int pid, int status){
    process* current = process_list;
    while (current != NULL) {
        if (current->pid == pid) {
            current->status = status;
            break;
        }
        current = current->next;
    }
}


void execute(cmdLine *pCmdLine) {
    pid_t processID;

    if(debugF==1){
        fprintf(stderr, "PID: %d\n", getpid());
        fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
    }


    int result = 1;

    if (strcmp( pCmdLine->arguments[0], "wakeup") == 0) {
        int pid = atoi(pCmdLine->arguments[1]);
        result = kill(pid, SIGCONT);

    }else if (strcmp( pCmdLine->arguments[0] , "suspend")==0){
        int pid = atoi(pCmdLine->arguments[1]);
        result = kill(pid, SIGSTOP);

    } else if (strcmp( pCmdLine->arguments[0], "nuke") == 0) {
        int pid = atoi(pCmdLine->arguments[1]);
        result = kill(pid, SIGKILL);
    }

    if (result == 0) {
        printf("Command '%s' sent successfully to process %s.\n",  pCmdLine->arguments[0], pCmdLine->arguments[1]);
        return;
    }
    if(pCmdLine->next != NULL){
        int pipefd[2];
        // Check for invalid redirections
        if (pCmdLine->outputRedirect != NULL) {
            fprintf(stderr, "Error: Output redirection in the first part of a pipeline is not allowed.\n");
            return;
        }
        if (pCmdLine->next->inputRedirect != NULL) {
            fprintf(stderr, "Error: Input redirection in the second part of a pipeline is not allowed.\n");
            return;
        }
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        processID = fork();
        //addProcess(&process_list, pCmdLine, processID);
        if (processID == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (processID == 0) { // Child process
            if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(pipefd[1]);
            close(pipefd[0]);
            if (execvp((*pCmdLine).arguments[0], pCmdLine->arguments) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else { // Parent process
            close(pipefd[1]);
            if (waitpid(processID, NULL, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            processID = fork();
           // addProcess(&process_list, pCmdLine->next, processID);
            if (processID == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (processID == 0) { // Child process
                if (dup2(pipefd[0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(pipefd[0]);
                if (execvp((*pCmdLine).next->arguments[0], pCmdLine->next->arguments) == -1) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            } else { // Parent process
                close(pipefd[0]);
                if (waitpid(processID, NULL, 0) == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }else{
        processID = fork();
       // addProcess(&process_list, pCmdLine, processID);
        if (processID == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (processID == 0) { // Child process
            if((*pCmdLine).inputRedirect != NULL){
                int input_file_desc = open((*pCmdLine).inputRedirect,O_RDWR);//open file  with read only access
                if (input_file_desc == -1) {
                    perror("open inputRedirect");
                    exit(EXIT_FAILURE);
                }
                if (dup2(input_file_desc, STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(input_file_desc);
            }

            if ((*pCmdLine).outputRedirect != NULL) {
                int input_file_desc = open((*pCmdLine).outputRedirect, O_WRONLY | O_CREAT | O_TRUNC);
                if (input_file_desc == -1) {
                    perror("open outputRedirect");
                    _exit(EXIT_FAILURE);
                }
                dup2(input_file_desc, STDOUT_FILENO);
                close(input_file_desc);
            }
            if (execvp((*pCmdLine).arguments[0], pCmdLine->arguments) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else { // Parent process
            //make a parent process wait until one of its child processes terminates.
            // If blocking is set (no "&" at the end), wait for the child process
            fprintf(stderr,"processCmd: %s\n",pCmdLine->arguments[0]);
            addProcess(&processList, pCmdLine, processID);
            if((*pCmdLine).blocking){
                if (waitpid(processID, NULL, 0) == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }
            }
        }

    }

}