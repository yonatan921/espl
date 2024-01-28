#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipe_fd[2];
    pid_t pid;
    char buffer[1024];

    // Create a pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        close(pipe_fd[0]);

        const char* message = "hello";
        if (write(pipe_fd[1], message, strlen(message)) == -1) {
            perror("write");
            close(pipe_fd[1]);
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[1]);

        exit(EXIT_SUCCESS);
    } else { // Parent process
        close(pipe_fd[1]);

        ssize_t bytesRead = read(pipe_fd[0], buffer, sizeof(buffer));

        if (bytesRead == -1) {
            perror("read");
            close(pipe_fd[0]);
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[0]);

        buffer[5] = '\0';

        printf("%s\n", buffer);
        if (waitpid(pid, NULL, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}