#include <stdio.h>
#include <sys/signal.h>

void
handler(int sig) {
    printf("pid: %d\n", getpid());
    exit(0);
}

int
main() {
    int term_pid = getppid();
    signal(SIGINT, handler);

    for (int i = 0; i < 4; i++) {
        fork();
        printf("i: %d pid: %d\n", i, getpid());
    }

    if (fork() == 0) {
        // setpgid(0, term_pid);
        // setpgid(0, 0);
        setpgid(0, 474587);
        printf("pgid: %d\n", getpgid());
    } else {
        printf("Hello from parent\n");
    }

    for (;;) {
        sleep(3);
    }
    return 0;
}