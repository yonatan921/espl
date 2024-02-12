
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


int main(int argc,char ** argv) {
    if ((argc > 1) && (strcmp(argv[1],"-d") == 0)) {
        debugF = 1;//debug on
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
            break;
        } else if (strncmp(input,"cd ", 3)==0) {
            char *path_cd = input + 3; // moving the pointer to extract the path after "cd "
            if (chdir(path_cd) == -1) {
                //chdir:This command returns zero (0) on success.
                // -1 is returned on an error and errno is set appropriately.
                fprintf(stderr, "cd: %s: Could not found such file or directory \n", path_cd);
            }
        }else if (strcmp(input, "procs") == 0) {
            printProcessList(&processList);
        }else{
            cmdLine *parsedCmd = parseCmdLines(input);// Parse the input
            execute(parsedCmd);// Execute the command
            //freeCmdLines(parsedCmd); // Release resources
        }


    }

    return 0;
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


//void printProcessList(process** process_list){
//   // updateProcessList(process_list);
//    process* current = *process_list;
//    printf("PID\tCommand\tSTATUS\n");
//    int i = 0;
//    while (current != NULL) {
//        char* status=(current->status==RUNNING)?"Running":(current->status==SUSPENDED)?"Suspended":"Terminated";
//        printf("%d\t%s\t%s\n",  current->pid, current->cmd->arguments[0], status);
//        current = current->next;
//        i++;
//    }
//}
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

void updateProcessList(process **process_list)
{
    int begin = 0;
    process* temp = *process_list;
    while(temp != NULL)
    {
        int returnID = waitpid(temp->pid, &begin, WNOHANG | WUNTRACED | WCONTINUED);
        if(WIFCONTINUED(begin)) // checks if the process changed its state to RUNNING
        {
            updateProcessStatus(temp, temp->pid, RUNNING);
            break;
        }
        else
        {
            if(WIFSTOPPED(begin)) // checks if the process changed its state to SUSPENDED
            {
                updateProcessStatus(temp, temp->pid, SUSPENDED);
                break;
            }
            else
            {
                if(returnID != 0) // checks if the process changed its state to TERMINATED
                    updateProcessStatus(temp, temp->pid, TERMINATED);
            }
        }
        temp = temp->next;
    }
}
//void updateProcessList(process **process_list) {
//    int stat = 0;
//    process* temp = *process_list;
//    while (temp != NULL) {
//        // Attempt to update the status of the process without blocking
//        int returnID = waitpid(temp->pid, &stat, WNOHANG | WUNTRACED | WCONTINUED);
//
//        if (returnID > 0) { // If waitpid returned, meaning there was a status change
//            if (WIFCONTINUED(stat)) {
//                // Process has continued running
//                temp->status = RUNNING;
//            } else if (WIFSTOPPED(stat)) {
//                // Process is stopped
//                temp->status = SUSPENDED;
//            } else if (WIFEXITED(stat) || WIFSIGNALED(stat)) {
//                // Process has exited or was killed
//                temp->status = TERMINATED;
//            }
//        }
//        // Move to the next process in the list
//        temp = temp->next;
//    }
//}

//void updateProcessList(process **process_list) {
//    int status;
//    process *current = *process_list, *prev = NULL;
//    while (current != NULL) {
//        if (waitpid(current->pid, &status, WNOHANG) > 0) {
//            if (WIFEXITED(status) || WIFSIGNALED(status)) {;
//                updateProcessStatus(*process_list, current->pid, TERMINATED);
//            } else if (WIFSTOPPED(status)) {
//                updateProcessStatus(*process_list, current->pid, SUSPENDED);
//            } else if (WIFCONTINUED(status)) {
//                updateProcessStatus(*process_list, current->pid, RUNNING);
//            }
//        }
//        prev = current;
//        current = current->next;
//    }
//}


//void updateProcessList(process **process_list){
//    process* current = *process_list;
//    process* prev = NULL;
//    while (current != NULL) {
//        int status;
//        int result = waitpid(current->pid, &status, WNOHANG);
//        if (result == -1) {
//            perror("waitpid");
//            exit(EXIT_FAILURE);
//        } else if (result == 0) {
//            prev = current;
//            current = current->next;
//        } else {
//            if (WIFEXITED(status)) {
//                current->status = TERMINATED;
//            } else if (WIFSTOPPED(status)) {
//                current->status = SUSPENDED;
//            } else if (WIFCONTINUED(status)) {
//                current->status = RUNNING;
//            }
//            if (prev == NULL) {
//                *process_list = current->next;
//                freeCmdLines(current->cmd);
//                free(current);
//                current = *process_list;
//            } else {
//                prev->next = current->next;
//                freeCmdLines(current->cmd);
//                free(current);
//                current = prev->next;
//            }
//        }
//    }
//}

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


/*Write a function execute(cmdLine *pCmdLine) that receives a parsed line and
 * invokes the program specified in the cmdLine using the proper system call . */

//Part 2: Implementing a Pipe in the Shell
//
//Having learned how to create a pipe between 2 processes/programs in Part 1, we now wish to implement a pipeline inside our own shell. In this part you will extend your shell's capabilities to support pipelines that consist of just one pipe and 2 child processes. That is, support a command line with one pipe between 2 processes resulting from running executable files mentioned in the command line. The scheme uses basically the same mechanism as in part 1, except that now the program to be executed in each child process is determined by the command line.
//Your shell must be able now to run commands like: ls|wc -l which basically counts the number of files/directories under the current working dir. The most important thing to remember about pipes is that the write-end of the pipe needs to be closed in all processes, otherwise the read-end of the pipe will not receive EOF, unless the main process terminates.
//
//Notes:
//    The line parser automatically generates a list of cmdLine structures to accommodate pipelines. For instance, when parsing the command "ls | grep .c", two chained cmdLine structures are created, representing ls and grep respectively.
//    Your shell must still support all previous features, including input/output redirection from lab 2. Obviously, it makes no sense to redirect the output of the left--hand-side process (as then nothing goes into the pipe), and this should be considered an error, and likewise redirecting the input of the right-hand-side process is an error (as then the pipe output is hanging). In such cases, print an error message to stderr without generating any new processes.
//    It is important to note that commands utilizing both I/O redirection and pipelines are indeed quite common (e.g. "cat < in.txt | tail -n 2 > out.txt").
//    As in previous tasks, you must keep your program free of memory leaks.
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