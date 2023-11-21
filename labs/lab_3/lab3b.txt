#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int
main() {
    int user_input;
    printf("Enter your choice: ");
    scanf("%d", &user_input);

    int pid;

    // fork the process and exit if error
    if ((pid = fork()) == -1) {
        printf("Error creating the fork...");
        exit(0);
    }

    if (pid == 0) { // child process
        switch (user_input) {
            case 1:
                // print pid and ppid and terminate normally
                printf("PID: %d, PPID: %d\n", getpid(), getppid());
                break;
            case 2:
                // print pid and ppid and exit with status 7
                printf("PID: %d, PPID: %d\n", getpid(), getppid());
                exit(7);
            case 3:
                // print pid and ppid 3 times with 1 second delay
                for (int i = 0; i < 3; i++) {
                    printf("PID: %d, PPID: %d\n", getpid(), getppid());
                    sleep(1);
                }
                int n = 100 / 0;
                break;
            default: break;
        }
    } else { // main process
        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                printf("Child process exited normally with status: %d\n",
                       WEXITSTATUS(status));
            } else if (WEXITSTATUS(status) == 7) {
                printf("Child process exited with status: %d\n",
                       WEXITSTATUS(status));
            }

        } else if (WIFSIGNALED(status)) {
            printf("Child process exited with signal: %d\n", WTERMSIG(status));
        }
    }

    return 0;
}