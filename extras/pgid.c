#include <signal.h>
#include <stdio.h>

int pgid[2] = {0, 0};

void
sigint_handler(int signo) {
    printf("In the handler\n");
    printf("pgid[0]: %d, pgid[1]: %d\n", pgid[0], pgid[1]);
    killpg(-1 * pgid[1], SIGKILL);
}

int
main() {
    signal(SIGINT, sigint_handler);
    pgid[0] = getpid();

    int pid;

    if ((pid = fork()) == 0) {
        pgid[1] = getpid();
        setpgid(0, 0);
    }

    printf("pgid[0]: %d, pgid[1]: %d\n", pgid[0], pgid[1]);

    pid = fork();
    printf("Fork 2: pid: %d, pgid: %d\n", getpid(), getpgid(0));
    if (pid == 0) {
        for (;;)
            ;
    } else {
        for (;;)
            ;
    }
}