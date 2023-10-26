#include <stdio.h>
#include <unistd.h>

int
main() {

    int pid;
    if ((pid = fork()) == -1) {
        perror("fork");
        return 1;
    }
    if ((pid = fork()) == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("exiting child pid: %d, PPID: %d\n", getpid(), getppid());
    } else {
        printf("parent pid: %d, ppid: %d\n", getpid(), getppid());
        for (;;)
            ;
    }

    return 0;
}