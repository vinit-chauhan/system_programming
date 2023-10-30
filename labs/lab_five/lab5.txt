#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <unistd.h>

// handler for second SIGINT
void
second_sigint_handler(int sig) {
    printf("Second handler\n");
    exit(0);
}

// handler for first SIGINT
void
first_sigint_handler(int sig) {
    printf("First handler\n");

    // set an alarm for 5 seconds
    alarm(5);
    // change to second handler
    signal(SIGINT, second_sigint_handler);
}

// if alarm goes off, revert the handler back to first handler
void
alarm_handler(int sig) {
    printf("Reverting back to first handler\n");
    // change back to default handler
    signal(SIGINT, first_sigint_handler);
}

int
main() {
    // set handler for SIGINT to use first handler
    signal(SIGINT, first_sigint_handler);
    // set handler for SIGALRM to use the alarm handler
    signal(SIGALRM, alarm_handler);

    // print the message after every 1 second.
    for (;;) {
        printf("Welcome to Lab5 - Signals\n");
        sleep(1);
    }
}