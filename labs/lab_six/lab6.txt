#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main_pgid, main_pid, is_gc_terminated;

// Handler for SIGINT
void
sigint_handler(int sig) {
    // Get the process ID
    int pid = getpid();

    // Change the value of is_gc_terminated
    is_gc_terminated = 1;

    // Check if it belongs to the same group and process ID is not the main_pid
    if (main_pgid == getpgid(0) && pid != main_pid && getppid() != main_pid) {
        setpgid(pid, getpid());
        // Kill the process
        kill(pid, SIGKILL);
    }
}

// Handler for SIGCONT
void
sigcont_handler(int signum) {
    printf("This is from process %d\n", getpid());
    sleep(2);
}

int
main() {
    // Initialize the SIGINT handler
    signal(SIGINT, sigint_handler);

    // Get main process' ID
    main_pid = getpid();

    // Get main process' group ID
    main_pgid = getpgid(0);

    // Two consecutive fork calls
    int pid1 = fork();
    int pid2 = fork();

    while (1) {
        // Print the process IDs
        printf("From process %d\n", getpid());

        // Sleep for 2 seconds
        sleep(2);

        if (is_gc_terminated == 1) {
            break;
        }
    }

    // For printing in sequence
    if (getpid() == main_pid) {
        while (1) {
            // After the termination statement
            printf("This is from process %d\n", getpid());
            sleep(2);

            // Resume the first child process
            kill(pid1, SIGCONT);
            // Sleep for 2 seconds
            sleep(2);
            // Suspend the first child process
            kill(pid1, SIGSTOP);

            // Resume the second child process
            kill(pid2, SIGCONT);
            // Sleep for 2 seconds
            sleep(2);
            // Suspend the second child process
            kill(pid2, SIGSTOP);
        }
    }

    // Print from the first child
    if (pid1 == 0) {
        // Set up the SIGCONT handler
        signal(SIGCONT, sigcont_handler);
        while (1)
            ;
    }

    // Print from the second child
    if (pid2 == 0) {
        // Set up the SIGCONT handler
        signal(SIGCONT, sigcont_handler);
        while (1)
            ;
    }

    return 0;
}
