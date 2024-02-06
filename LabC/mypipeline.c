//Note
//This part is independent of the shell, and a preparation for implementing a pipe command in the shell. You should not use the LineParser functions in this task, nor read any command lines. However, you need to declare an array of "strings" containing all of the arguments and ending with 0 to pass to execvp() just like the one returned by parseCmdLines().
//
//
//Here we wish to explore the implementation of a pipeline. In order to achieve such a pipeline, one has to create a pipe and properly redirect the standard output and standard input of processes.
//Please refer to the 'Introduction to Pipelines' section in the reading material.
//
//Your task: Write a short program called mypipeline which creates a pipeline of 2 child processes. Essentially, you will implement the shell command line "ls -l | tail -n 2".
//(A question: what does "ls -l" do, what does "tail -n 2" do, and what should their combination produce?)
//
//Follow the given steps as closely as possible to avoid synchronization problems:
//
//    Create a pipe.
//    Fork a first child process (child1).
//    In the child1 process:
//        Close the standard output.
//        Duplicate the write-end of the pipe using dup (see man).
//        Close the file descriptor that was duplicated.
//        Execute "ls -l".
//    In the parent process: Close the write end of the pipe.
//    Fork a second child process (child2).
//    In the child2 process:
//        Close the standard input.
//        Duplicate the read-end of the pipe using dup.
//        Close the file descriptor that was duplicated.
//        Execute "tail -n 2".
//    In the parent process: Close the read end of the pipe.
//    Now wait for the child processes to terminate, in the same order of their execution.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

 int main(int argc,char ** argv) {
    int pipefd[2];
    pid_t cpid1,cpid2;
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    cpid1 = fork();
    if (cpid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (cpid1 == 0) {    /* Child 1: ls -l */
        close(pipefd[0]);          /* Close unused read end */
        dup2(pipefd[1], STDOUT_FILENO);   /* Duplicate write end to stdout */
        close(pipefd[1]);          /* Close write end */
        execlp("ls", "ls", "-l", (char *) NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {            /* Parent */
        close(pipefd[1]);          /* Close unused write end */
        cpid2 = fork();
        if (cpid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (cpid2 == 0) {    /* Child 2: tail -n 2 */
            dup2(pipefd[0], STDIN_FILENO);   /* Duplicate read end to stdin */
            close(pipefd[0]);          /* Close read end */
            execlp("tail", "tail", "-n", "2", (char *) NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        } else {            /* Parent */
            close(pipefd[0]);          /* Close unused read end */
            waitpid(cpid1, NULL, 0);
            waitpid(cpid2, NULL, 0);
        }
    }
    return 0;
}