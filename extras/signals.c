#include <stdio.h>
#include <sys/signal.h>

void
sigint_handler(int sig) {
    printf("Immune to Ctrl+C\n");
}

void (*old_func)(int);

void
alarm_handler(int sig) {
    printf("Reverting back to default handler\n");
    signal(SIGINT, old_func);
}

int
main() {
    alarm(5);
    old_func = signal(SIGINT, sigint_handler);
    signal(SIGALRM, alarm_handler);
    for (;;)
        ;
}