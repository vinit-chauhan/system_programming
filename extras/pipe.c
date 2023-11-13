#include <stdio.h>
#include <unistd.h>

int
main() {
    int pipefd[2], pid1, pid2;

    pipe(pipefd);

    if ((pid1 = fork()) == -1) {
        perror("fork");
        return 1;
    }

    if (pid1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execlp("ls", "ls", "-l", NULL);
        perror("ls");
        return 1;
    } else {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        execlp("wc", "wc", "-l", NULL);
    }

    return 0;
}