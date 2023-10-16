#include <stdio.h>
#include <stdlib.h>

int
main() {
    int first_pid, second_pid;
    // create first fork
    if ((first_pid = fork()) == -1) {
        return -1;
    }
    // create second fork
    if ((second_pid = fork()) == -1) {
        return -1;
    }

    if (first_pid > 0 && second_pid > 0) { // main process
        printf("The PID and PPID of the main process is: \t\t%d, %d\n",
               getpid(), getppid());
    } else if (first_pid == 0 && second_pid > 0) { // first child
        printf("The PID and PPID of the second first process is:\t%d, %d\n",
               getpid(), getppid());
    } else if (first_pid > 0 && second_pid == 0) { // second child
        printf("The PID and PPID of the second child process is: \t%d, %d\n",
               getpid(), getppid());
    } else if (first_pid == 0 && second_pid == 0) { // grand child
        printf("The PID and PPID of the grand-child process is: \t%d, %d\n",
               getpid(), getppid());
    }

    return 0;
}