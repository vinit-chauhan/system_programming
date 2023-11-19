#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

enum command_operation {
    NOOP,
    PIPE,
    LOGICAL_AND,
    LOGICAL_OR,
    BACKGROUND,
    CHAIN
};

enum command_sub_operation {
    NO_SUB_OP,
    REDIRECT_IN,
    REDIRECT_OUT,
    REDIRECT_APPEND
};

typedef struct {
    char* c_full_command;
    char* c_args[4];
    char c_name[32];
    int c_argv;
    enum command_sub_operation c_sub_operation;
} t_command;

#define INIT_COMMAND(t_command)                                                \
    t_command.c_argv = 0;                                                      \
    t_command.c_sub_operation = NO_SUB_OP;

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
process_command(char* cmd, t_command* command) {
    char* tmp;
    char* start = cmd;
    for (int i = 0; i < strlen(cmd); i++) {
        if (cmd[i] == ' ') {
            memcpy(tmp, start, i);
            start = cmd + i + 1;
        }

        if (cmd[i] == '\0') {
            memcpy(tmp, start, i);
            break;
        }

        if (cmd[i] == '<') {
            command->c_sub_operation = REDIRECT_IN;
        } else if (cmd[i] == '>') {
            command->c_sub_operation = REDIRECT_OUT;
            if (cmd[i + 1] == '>') {
                command->c_sub_operation = REDIRECT_APPEND;
            }
        }
    }
}

void
process_line(char* line, t_command* commands, int* operations,
             int command_count) {
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
            operations[i] = BACKGROUND;
            if (line[i + 1] == '&') {
                operations[i] = LOGICAL_AND;
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
        int* operations;
        int command_count = 0;
        size_t line_size = 0;

        char* token = malloc(1024 * sizeof(char));

        printf("mybash$ {%d} : ", getpid());
        fflush(stdout);

        getline(&line, &line_size, stdin);

        if (strcmp(line, "exit\n") == 0) {
            break;
        }
        for (int i = 0; i < strlen(line); i++) {
            if (line[i] == '\n') {
                line[i] = '\0';
            }

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

        process_line(line, commands, operations, command_count);

        char* cmd = strtok(line, "|&&;");
        int i = 0;
        while (cmd != NULL) {
            printf("commands[%d].c_full_command : %s\n", i,
                   commands[i].c_full_command);
            // process_command(cmd, &commands[i]);
            cmd = strtok(NULL, "|&&;");
            i++;
        }
    }
    return 0;
}