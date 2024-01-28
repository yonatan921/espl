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

    // Create a child process
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        // Close the read end of the pipe in the child
        close(pipe_fd[0]);

        // Write a message to the pipe
        const char* message = "hello";
        if (write(pipe_fd[1], message, strlen(message)) == -1) {
            perror("write");
            close(pipe_fd[1]);
            exit(EXIT_FAILURE);
        }

        // Close the write end of the pipe
        close(pipe_fd[1]);

        exit(EXIT_SUCCESS);
    } else { // Parent process
        // Close the write end of the pipe in the parent
        close(pipe_fd[1]);

        // Read from the pipe
        ssize_t bytesRead = read(pipe_fd[0], buffer, sizeof(buffer));

        if (bytesRead == -1) {
            perror("read");
            close(pipe_fd[0]);
            exit(EXIT_FAILURE);
        }

        // Close the read end of the pipe
        close(pipe_fd[0]);

        // Print the received message
        printf("Received message from child: %.*s\n", (int)bytesRead, buffer);

        // Wait for the child to exit
        if (wait(NULL) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}