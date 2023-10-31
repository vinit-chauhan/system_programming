#include <stdio.h>
#include <unistd.h>

int
main() {

    int pid1, pid2;
    if ((pid1 = fork()) == -1) {
        perror("fork");
        return 1;
    }
    if ((pid2 = fork()) == -1) {
        perror("fork");
        return 1;
    }

    if (pid1 != 0 && pid2 != 0) {
        printf("parent pid: %d, ppid: %d\n", getpid(), getppid());
        for (;;)
            ;
    } else {
        printf("exiting child pid: %d, PPID: %d\n", getpid(), getppid());
        int pid = fork();
        if (pid == 0) {
            printf("parent pid: %d, ppid: %d\n", getpid(), getppid());
            int pid = fork();
            if (pid == 0) {
                printf("parent pid: %d, ppid: %d\n", getpid(), getppid());

            } else {
                for (;;)
                    ;
            }
        } else {
            wait();
        }
    }

    return 0;
}