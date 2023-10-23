#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int
main() {

    int first_child_pid, second_child_pid;

    if ((first_child_pid = fork()) == -1) {
        printf("Error creating first child process\n");
        exit(1);
    }
    if ((second_child_pid = fork()) == -1) {
        printf("Error creating second child process\n");
        exit(1);
    }
    char* processName = "/bin/ls";
    char* arg1 = "-1";
    char* home_dir = "/home/vinit-chauhan/";
    char* lab_dir = "/home/vinit-chauhan/lab4";

    if (first_child_pid > 0 && second_child_pid > 0) { // main process
        // printf("Parent process\n");
    } else if (first_child_pid > 0 && second_child_pid == 0) { // second child
        // printf("Second child process\n");
        execl(processName, processName, arg1, home_dir, NULL);
    } else if (first_child_pid == 0 && second_child_pid > 0) { // first child
        // printf("First child process\n");
        // waiting for grand child to finish
        waitpid(second_child_pid, NULL, 0);
        char* args[] = {processName, arg1, lab_dir, NULL};
        // do ls -1 to lab4 directory
        execv(processName, args);
    } else if (first_child_pid == 0 && second_child_pid == 0) { // grand child
        umask(0);
        // printf("Grand child process\n");
        // changing directory to /home/vinit-chauhan/lab4
        chdir("/home/vinit-chauhan/lab4");
        // creating a file sample.txt
        int fd = open("sample.txt", O_CREAT, 0777);
        if (fd == -1) { // Check of any error in file creation.
            printf("Error creating file\n");
            exit(1);
        } else {
            printf("File created successfully\n");
        }
    }

    return 0;
}