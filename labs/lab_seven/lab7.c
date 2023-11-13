#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int
main() {
    int pipefd[2], pipefd2[2], pid, pid1;
    if ((pipe(pipefd)) == -1) {
        perror("pipe");
        return 1;
    }

    int output_fd = open("lab7output.txt", O_CREAT | O_WRONLY, 0666);

    if ((pid = fork()) == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        dup2(pipefd[1], 1);
        close(pipefd[0]);
        execlp("cat", "cat", "f23.txt", NULL);
        perror("ls");
    } else {
        if ((pipe(pipefd2)) == -1) {
            perror("pipe");
            return 1;
        }
        if ((pid1 = fork()) == -1) {
            perror("fork");
            return 1;
        }
        if (pid1 == 0) {
            close(pipefd[1]);
            close(pipefd2[0]);
            dup2(pipefd[0], 0);
            dup2(pipefd2[1], 1);
            execlp("grep", "grep", "Welcome", NULL);
            perror("grep");
        }

        if ((pid = fork()) == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            waitpid(pid1, NULL, 0);
            dup2(pipefd2[0], 0);
            dup2(output_fd, 1);
            execlp("wc", "wc", NULL);
            perror("wc");
        }
    }

    return 0;
}