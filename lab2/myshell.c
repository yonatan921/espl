#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define STDIN_FAILURE -1
#define MAX_SIZE 2048

int debugF=0; // zero means debug off - have to be global , otherwise signatures of the functions will need to change.
void execute(cmdLine *pCmdLine);

int main(int argc,char ** argv) {
    if ((argc > 1) && (strcmp(argv[1],"-d") == 0)) {
        debugF = 1;//debug on
    }

    while (1) {
        char input[MAX_SIZE];
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current directory: %s\n", cwd);
        } else {
            perror("getcwd did not work!");
            exit(EXIT_FAILURE);
        }

        printf("Enter command: ");
        if (fgets(input, MAX_SIZE, stdin) == NULL) {//T0a2
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        // Remove the newline character at the end
        size_t input_len = strlen(input);
        if (input_len > 0 && input[0] == '\n') {
            continue;
        }
        if (input_len > 0 && input[input_len - 1] == '\n') {
            input[input_len - 1] = '\0';
        }
        if (strcmp(input, "quit") == 0) {
            printf("Exit shell.\n");
            break;
        } else if (strncmp(input,"cd ", 3)==0){
            char *path_cd = input + 3; // moving the pointer to extract the path after "cd "
            if (chdir(path_cd) == -1) {
                //chdir:This command returns zero (0) on success.
                // -1 is returned on an error and errno is set appropriately.
                fprintf(stderr, "cd: %s: Could not found such file or directory \n", path_cd);
            }
        }else{

            cmdLine *parsedCmd = parseCmdLines(input);// Parse the input
            execute(parsedCmd);// Execute the command
            freeCmdLines(parsedCmd); // Release resources
        }


    }

    return 0;
}

/*Write a function execute(cmdLine *pCmdLine) that receives a parsed line and
 * invokes the program specified in the cmdLine using the proper system call . */
void execute(cmdLine *pCmdLine) {
    pid_t processID;

    if(debugF==1){
        fprintf(stderr, "PID: %d\n", getpid());
        fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
    }

    int result = 1;

    // Signal handling
    if (strcmp( pCmdLine->arguments[0], "wakeup") == 0) {

        int pid = atoi(pCmdLine->arguments[1]);
        printf("%d\n", pid);
        result = kill(pid, SIGCONT);


    } else if (strcmp( pCmdLine->arguments[0], "nuke") == 0) {
        int pid = atoi(pCmdLine->arguments[1]);
        result = kill(pid, SIGINT);

    }

    if (result == 0) {
        printf("Command '%s' sent successfully to process %s.\n",  pCmdLine->arguments[0], pCmdLine->arguments[1]);
        return;
    }


    processID = fork();


    if (processID == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }


    if (processID == 0) { // Child process
        //if input redirect !=null
        if((*pCmdLine).inputRedirect != NULL){
            printf("input");
            int input_file_desc = open((*pCmdLine).inputRedirect,O_RDWR );//open file  with read only access
            if (input_file_desc == -1){
                perror("open inputRedirect");
                exit(EXIT_FAILURE);
            }
            printf("here");
            dup2(input_file_desc,STDIN_FILENO);
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
        if((*pCmdLine).blocking){
            if (waitpid(processID, NULL, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
        }
    }


}