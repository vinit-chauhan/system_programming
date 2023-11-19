#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

enum command_operation {
    SINGLE_COMMAND,
    COMMAND_WITH_PIPE,
    COMMAND_WITH_REDIRECT_IN,
    COMMAND_WITH_REDIRECT_OUT,
    COMMAND_WITH_REDIRECT_APPEND,
    COMMAND_WITH_LOGICAL_AND,
    COMMAND_WITH_LOGICAL_OR,
    COMMAND_WITH_BACKGROUND,
    COMMAND_WITH_SEMICOLON
};

typedef struct {
    char* c_args[4];
    char c_name[32];
    int c_argv;
    enum command_operation c_operation;
} command;

#define INITIALIZE_COMMAND(command)                                            \
    command.c_argv = 0;                                                        \
    command.c_operation = SINGLE_COMMAND;

int
main(int argc, char* argv[]) {
    int stdin_fd = dup(STDIN_FILENO);
    int stdout_fd = dup(STDOUT_FILENO);

    char* line = malloc(1024 * sizeof(char));

    while (1) {
        command commands[3];
        INITIALIZE_COMMAND(commands[0]);
        INITIALIZE_COMMAND(commands[1]);
        INITIALIZE_COMMAND(commands[2]);
        int command_count = 0;
        size_t line_size = 0;
        char* token = malloc(1024 * sizeof(char));

        printf("mybash$ {%d} : ", getpid());
        fflush(stdout);

        getline(&line, &line_size, stdin);

        if (strcmp(line, "exit\n") == 0) {
            break;
        }

        token = strtok(line, " \n");
        while (token != NULL) {
            if (strcmp(token, "|") == 0) {
                commands[command_count].c_args[commands[command_count].c_argv] =
                    NULL;
                commands[command_count].c_operation = COMMAND_WITH_PIPE;
                command_count++;
            } else if (strcmp(token, "<") == 0) {
                commands[command_count].c_args[commands[command_count].c_argv] =
                    NULL;
                commands[command_count].c_operation = COMMAND_WITH_REDIRECT_IN;
                command_count++;
            } else if (strcmp(token, ">") == 0) {
                commands[command_count].c_args[commands[command_count].c_argv] =
                    NULL;
                commands[command_count].c_operation = COMMAND_WITH_REDIRECT_OUT;
                command_count++;
            } else if (strcmp(token, ">>") == 0) {
                commands[command_count].c_args[commands[command_count].c_argv] =
                    NULL;
                commands[command_count].c_operation =
                    COMMAND_WITH_REDIRECT_APPEND;
                command_count++;
            } else if (strcmp(token, "&&") == 0) {
                commands[command_count].c_args[commands[command_count].c_argv] =
                    NULL;
                commands[command_count].c_operation = COMMAND_WITH_LOGICAL_AND;
                command_count++;
            } else if (strcmp(token, "||") == 0) {
                commands[command_count].c_args[commands[command_count].c_argv] =
                    NULL;
                commands[command_count].c_operation = COMMAND_WITH_LOGICAL_OR;
                command_count++;
            } else if (strcmp(token, "&") == 0) {
                commands[command_count].c_args[commands[command_count].c_argv] =
                    NULL;
                commands[command_count].c_operation = COMMAND_WITH_BACKGROUND;
                command_count++;
            } else if (strcmp(token, ";") == 0) {
                commands[command_count].c_args[commands[command_count].c_argv] =
                    NULL;
                commands[command_count].c_operation = COMMAND_WITH_SEMICOLON;
                command_count++;
            } else {
                if (commands[command_count].c_argv == 0) {
                    strcpy(commands[command_count].c_name, token);
                }
                commands[command_count].c_args[commands[command_count].c_argv] =
                    token;
                commands[command_count].c_argv++;
            }
            token = strtok(NULL, " \n");
        }

        commands[command_count].c_args[commands[command_count].c_argv] = NULL;
        int pid;
        if ((pid = fork()) == -1) {
            perror("fork");
            return -1;
        } else if (pid > 0) {
            wait(NULL);
        } else {
            if (commands[0].c_operation == SINGLE_COMMAND) {
                execvp(commands[0].c_name, commands[0].c_args);
            } else {
                if (commands[0].c_operation == COMMAND_WITH_PIPE) {
                    int pipefd[2];
                    if (pipe(pipefd) == -1) {
                        perror("pipe");
                        return -1;
                    }

                    int pid1;
                    if ((pid1 = fork()) == -1) {
                        perror("fork");
                        return -1;
                    } else if (pid1 > 0) {
                        int pid2;
                        if ((pid2 = fork()) == -1) {
                            perror("fork");
                            return -1;
                        } else if (pid2 > 0) {
                            close(pipefd[1]);
                            dup2(pipefd[0], STDIN_FILENO);
                            close(pipefd[0]);
                            execvp(commands[1].c_name, commands[1].c_args);
                        }
                    } else {
                        close(pipefd[0]);
                        dup2(pipefd[1], STDOUT_FILENO);
                        close(pipefd[1]);
                        execvp(commands[0].c_name, commands[0].c_args);
                    }
                } else if (commands[0].c_operation
                           == COMMAND_WITH_REDIRECT_IN) {
                    int fd = open(commands[1].c_name, O_RDONLY);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    execvp(commands[0].c_name, commands[0].c_args);
                } else if (commands[0].c_operation
                           == COMMAND_WITH_REDIRECT_OUT) {
                    int fd = open(commands[1].c_name, O_WRONLY | O_CREAT, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execvp(commands[0].c_name, commands[0].c_args);
                } else if (commands[0].c_operation
                           == COMMAND_WITH_REDIRECT_APPEND) {
                    int fd = open(commands[1].c_name, O_WRONLY | O_APPEND);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execvp(commands[0].c_name, commands[0].c_args);
                } else if (commands[0].c_operation
                           == COMMAND_WITH_LOGICAL_AND) {
                    int pid1;
                    if ((pid1 = fork()) == -1) {
                        perror("fork");
                        return -1;
                    } else if (pid1 > 0) {
                        perror("execvp");
                        exit(-1);
                    }
                }
            }
        }
    }
    return 0;
}