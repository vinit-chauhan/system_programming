#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int BACKGROUND = 0;

enum command_operation { NOOP, PIPE, LOGICAL_AND, LOGICAL_OR, CHAIN };

enum command_sub_operation {
    NO_SUB_OP,
    REDIRECT_IN,
    REDIRECT_OUT,
    REDIRECT_APPEND
};

typedef struct {
    char* c_full_command;
    char* c_args[4];
    char* c_name;
    int c_argv;
    char* c_redirection_file;
    enum command_operation c_operation;
    enum command_sub_operation c_sub_operation;
} t_command;

#define INIT_COMMAND(t_command)                                                \
    t_command.c_argv = 0;                                                      \
    t_command.c_sub_operation = NO_SUB_OP;                                     \
    t_command.c_operation = NOOP;                                              \
    t_command.c_redirection_file = NULL;                                       \
    t_command.c_full_command = NULL;                                           \
    t_command.c_name = NULL;                                                   \
    memset(t_command.c_args, 0, 4 * sizeof(char*));

char*
trim(char* str) {
    char* start = str;
    char* end = strlen(str) + str - 1;

    while (*start == ' ') {
        start++;
    }
    while (*end == ' ' || *end == '&' || *end == '|') {
        end--;
    }
    *(end + 1) = '\0';

    return strdup(start);
}

void
print_command(t_command* command) {
    printf("full command: %s\n", command->c_full_command);
    printf("command name: %s\n", command->c_args[0]);
    printf("command args: ");
    for (int i = 0; i < command->c_argv; i++) {
        printf("%s ", command->c_args[i]);
    }
    printf("\n");
    printf("command argv: %d\n", command->c_argv);
    printf("command operation: %d\n", command->c_operation);
    printf("command sub operation: %d\n", command->c_sub_operation);
}

void
process_command(t_command* command) {
    char* cmd = strdup(command->c_full_command);
    char* token = strtok(cmd, " ");
    int count = 0;

    while (token != NULL) {
        command->c_args[count] = strdup(token);
        count++;

        if (strcmp(token, ">") == 0) {
            command->c_sub_operation = REDIRECT_OUT;
            token = strtok(NULL, " ");
            command->c_redirection_file = token;
            break;
        } else if (strcmp(token, ">>") == 0) {
            command->c_sub_operation = REDIRECT_APPEND;
            token = strtok(NULL, " ");
            command->c_redirection_file = token;
            break;
        } else if (strcmp(token, "<") == 0) {
            command->c_sub_operation = REDIRECT_IN;
            token = strtok(NULL, " ");
            command->c_redirection_file = token;
            break;
        }
        token = strtok(NULL, " ");
    }
    command->c_name = command->c_args[0];
    command->c_args[count] = NULL;
    command->c_argv = count;

    if (count > 3) {
        printf("mybash: %s: too many arguments\n", command->c_name);
        exit(1);
    }
}

void
process_line(char* line, t_command* commands, int* operations) {
    // parse the line and store the commands and operations
    char* tmp = malloc(1024 * sizeof(char));
    int flag = 0;
    for (int i = 0, j = 0, start = 0; i < strlen(line); i++) {
        if (line[i] == '|') {
            operations[i] = PIPE;
            if (line[i + 1] == '|') {
                operations[i] = LOGICAL_OR;
                i++;
            }
            flag = 1;
        } else if (line[i] == '&') {
            BACKGROUND = 1;
            if (line[i + 1] == '&') {
                operations[i] = LOGICAL_AND;
                BACKGROUND = 0;
                i++;
            }
            flag = 1;
        } else if (line[i] == ';') {
            operations[i] = CHAIN;
            flag = 1;
        } else if (line[i] == '\0' || line[i] == '\n'
                   || i == strlen(line) - 1) {
            flag = 2;
        }

        if (flag == 1) {
            memcpy(tmp, line + start, i - start);
            tmp[i - start] = '\0';
            commands[j].c_full_command = trim(tmp);
            commands[j].c_operation = operations[i];
            j++;
            start = i + 1;
            flag = 0;
        } else if (flag == 2) {
            memcpy(tmp, line + start, strlen(line) - start);
            tmp[strlen(line) - start] = '\0';
            commands[j].c_full_command = trim(tmp);
        }
    }
}

int
main(int argc, char* argv[]) {
    int stdin_fd = dup(STDIN_FILENO);
    int stdout_fd = dup(STDOUT_FILENO);

    char* line = malloc(1024 * sizeof(char));

    while (1) {
        t_command* commands;
        int pipes_pool[64][2];
        int* pid_pool;
        int* operations;
        int command_count = 1;
        size_t line_size = 0;

        char* token = malloc(1024 * sizeof(char));
        dup2(stdin_fd, STDIN_FILENO);
        dup2(stdout_fd, STDOUT_FILENO);

        // TODO: Remove pid from here.
        printf("mybash$ {%d} : ", getpid());
        fflush(stdout);

        getline(&line, &line_size, stdin);

        if (strcmp(line, "exit\n") == 0) {
            exit(0);
        }

        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        for (int i = 0; i < strlen(line) - 1; i++) {

            if (line[i] == '|' || line[i] == '&' || line[i] == ';') {
                command_count++;
            }
        }

        commands = malloc(command_count * sizeof(t_command));
        operations = malloc(command_count * sizeof(int));

        for (int i = 0; i < command_count; i++) {
            INIT_COMMAND(commands[i]);
            operations[i] = NOOP;
        }

        process_line(line, commands, operations);

        pid_pool = malloc(command_count * sizeof(int));
        for (int i = 0; i < command_count; i++) {
            process_command(&commands[i]);
            if (pipe(pipes_pool[i]) < 0) {
                printf("pipe failed\n");
                exit(1);
            }
        }

        for (int i = 0; i < command_count; i++) {
            if (commands[i].c_operation == PIPE
                || commands[i].c_operation == NOOP) {
                pid_pool[i] = fork();
                if (pid_pool[i] < 0) {
                    perror("fork failed\n");
                } else if (pid_pool[i] == 0) {
                    if (i != 0) {
                        dup2(pipes_pool[i - 1][0], STDIN_FILENO);
                        close(pipes_pool[i - 1][0]);
                    }
                    if (i != command_count - 1) {
                        dup2(pipes_pool[i][1], STDOUT_FILENO);
                        close(pipes_pool[i][1]);
                    }
                    for (int j = 0; j < command_count; j++) {
                        close(pipes_pool[j][0]);
                        close(pipes_pool[j][1]);
                    }

                    if (execvp(commands[i].c_name, commands[i].c_args) < 0) {
                        printf("mybash: %s: command not found\n",
                               commands[i].c_name);
                        exit(1);
                    }
                }
            }
        }

        for (int j = 0; j < command_count; j++) {
            close(pipes_pool[j][0]);
            close(pipes_pool[j][1]);
        }

        for (int j = 0; j < command_count; j++) {
            if (!BACKGROUND) {
                waitpid(pid_pool[j], NULL, 0);
            } else {
                printf("Process running in background with process id : %d \n",
                       pid_pool[j]);
            }
        }
    }
    return 0;
}