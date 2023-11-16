#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(int argc, char* argv[]) {

    int stdout_fd = dup(1);
    int stdin_fd = dup(0);

    char* line = NULL;

    while (1) {
        printf("mybash$ ");
        fflush(stdout);

        size_t len = 0;
        size_t read = getline(&line, &len, stdin);

        if (read == -1) {
            break;
        }

        if (strcmp(line, "exit\n") == 0) {
            break;
        }

        char* token = strtok(line, " \n");
        char* args[100];
        int i = 0;
        while (token != NULL) {
            args[i] = token;
            token = strtok(NULL, " \n");
            i++;
        }
        args[i] = NULL;

        int pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            printf("mybash: %s: command not found\n", args[0]);
            fflush(stdout);
            return 0;
        } else {
            wait(NULL);
        }
    }

    return 0;
}