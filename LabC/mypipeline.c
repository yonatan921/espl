#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    char * arguments[] ={"ls", "-l", "tail", "-n", "2"};
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "(parent_process>forking…)\n");
    pid_t child1 = fork();
    fprintf(stderr,"(parent_process>created process with id: %d)\n",child1);
    if (child1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (child1 == 0) {
        // In the child1 process:
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        fprintf(stderr,"(parent_process>closing the write end of the pipe…)\n");
        close(pipefd[1]);
        char *cmd[] = {arguments[0], arguments[1], NULL};
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
        fprintf(stderr, "(child1>going to execute cmd: %s\t%s\n",arguments[0],arguments[1]);
        execvp(cmd[0], cmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    close(pipefd[1]);
    pid_t child2 = fork();
    if (child2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (child2 == 0) {
        // In the child2 process:
        close(STDIN_FILENO);
        dup(pipefd[0]);
        fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
        close(pipefd[0]);
        char *cmd[] = {arguments[2], arguments[3], arguments[4], NULL};
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
        fprintf(stderr, "(child2>going to execute cmd: %s\t%s\t%s\n",arguments[2],arguments[3],arguments[4]);
        execvp(cmd[0], cmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    close(pipefd[0]);
    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    fprintf(stderr, "(parent_process>exiting…)\n");
    return 0;
}
