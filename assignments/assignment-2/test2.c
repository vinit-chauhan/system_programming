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

    if (pid1 == 0) {
        sleep(2);
    } else {
        fork();
        for (;;)
            ;
    }

    return 0;
}