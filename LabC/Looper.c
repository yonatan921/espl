#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

void signal_handler(int signum);

int main(int argc, char **argv) {
    printf("Starting the program\n");
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCONT, signal_handler);

    while (1) {
        sleep(1);
    }

    return 0;
}

void signal_handler(int signum) {
    printf("\nReceived Signal: %s\n", strsignal(signum));

    if (signum == SIGTSTP) {
        signal(SIGTSTP, SIG_DFL);
    } else if (signum == SIGCONT) {
        signal(SIGCONT, SIG_DFL);
    }

    signal(signum, SIG_DFL);
    raise(signum);

    // After handling SIGCONT, make sure you reinstate the custom handler for SIGTSTP
    if (signum == SIGCONT) {
        signal(SIGTSTP, signal_handler);
    }

    // After handling SIGTSTP, make sure you reinstate the custom handler for SIGCONT
    if (signum == SIGTSTP) {
        signal(SIGCONT, signal_handler);
    }
}