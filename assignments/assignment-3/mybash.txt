// Name: Vinit Hemantbhai Chauhan
// Student Id: 110123359
// Assignment 3 : Section 1

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int BACKGROUND = 0;

// Enum for command operations
enum command_operation { NOOP, PIPE, LOGICAL_AND, LOGICAL_OR, CHAIN };

// Enum for command sub operations
enum command_sub_operation {
    NO_SUB_OP,
    REDIRECT_IN,
    REDIRECT_OUT,
    REDIRECT_APPEND
};

// Struct to store the command
typedef struct {
    char* c_full_command;
    char* c_args[4];
    char* c_name;
    int c_argv;
    char* c_redirection_file;
    enum command_operation c_operation;
    enum command_sub_operation c_redirection_operation;
} command_t;

// Macro to initialize new command struct
#define INIT_COMMAND(command_t)                                                \
    command_t.c_argv = 0;                                                      \
    command_t.c_redirection_operation = NO_SUB_OP;                             \
    command_t.c_operation = NOOP;                                              \
    command_t.c_redirection_file = NULL;                                       \
    command_t.c_full_command = NULL;                                           \
    command_t.c_name = NULL;                                                   \
    memset(command_t.c_args, 0, 4 * sizeof(char*));

// Trim the whitespaces from beginning and end of the the string
char*
trim(char* str) {
    char* start = str;
    char* end = strlen(str) + str - 1;

    // remove trailing & and | from && and ||
    if (*end == '&' || *end == '|') {
        end--;
    }

    // remove leading spaces
    while (*start == ' ' || *start == '\t') {
        start++;
    }

    // remove trailing spaces
    while (*end == ' ' || *end == '\t') {
        end--;
    }

    // add null character at the end
    *(end + 1) = '\0';

    return strdup(start);
}

// Print the struct of command for debugging
void
print_command(command_t* command) {
    printf("full command: %s\n", command->c_full_command);
    printf("command name: %s\n", command->c_args[0]);
    printf("command args: ");
    for (int i = 0; i < command->c_argv; i++) {
        printf("%s ", command->c_args[i]);
    }
    printf("\n");
    printf("command argv: %d\n", command->c_argv);
    printf("command operation: %d\n", command->c_operation);
    printf("command redirection operation: %d\n",
           command->c_redirection_operation);
    printf("command redirection file: %s\n", command->c_redirection_file);
}

// Process the command and store the arguments
void
process_command(command_t* command) {
    char* cmd = strdup(command->c_full_command);
    char* token = strtok(cmd, " ");
    int count = 0;

    // Iterate over the command and store the arguments
    while (token != NULL) {
        command->c_args[count] = strdup(token);
        count++;

        // Set the appropriate redirection operation
        if (strcmp(token, ">") == 0) {
            command->c_redirection_operation = REDIRECT_OUT;
            token = strtok(NULL, " ");
            command->c_redirection_file = token;
            count--;
            break;
        } else if (strcmp(token, ">>") == 0) {
            command->c_redirection_operation = REDIRECT_APPEND;
            token = strtok(NULL, " ");
            command->c_redirection_file = token;
            count--;
            break;
        } else if (strcmp(token, "<") == 0) {
            command->c_redirection_operation = REDIRECT_IN;
            token = strtok(NULL, " ");
            command->c_redirection_file = token;
            count--;
            break;
        }
        token = strtok(NULL, " ");
    }
    command->c_name = command->c_args[0];
    command->c_args[count] = NULL;
    command->c_argv = count;

    // Check if the command has more than 3 arguments
    if (count > 3) {
        printf("%s: too many arguments\n", command->c_args[0]);
    }
}

// Process the line and store the commands and operations
void
process_line(char* line, command_t* commands) {
    // parse the line and store the commands and operations
    char* tmp = malloc(1024 * sizeof(char));
    int flag = 0;
    for (int i = 0, j = 0, start = 0; i < strlen(line); i++) {
        enum command_operation operation = NOOP;
        if (line[i] == '|') {
            // TODO: remove operations array from the code.
            operation = PIPE;
            if (line[i + 1] == '|') {
                operation = LOGICAL_OR;
                i++;
            }
            flag = 1;
        } else if (line[i] == '&') {
            BACKGROUND = 1;
            if (line[i + 1] == '&') {
                operation = LOGICAL_AND;
                BACKGROUND = 0;
                i++;
            }
            flag = 1;
        } else if (line[i] == ';') {
            operation = CHAIN;
            flag = 1;
        } else if (line[i] == '\0' || line[i] == '\n'
                   || i == strlen(line) - 1) {
            flag = 2;
        }

        if (flag == 1) {
            memcpy(tmp, line + start, i - start);
            tmp[i - start] = '\0';
            commands[j].c_full_command = trim(tmp);
            commands[j].c_operation = operation;
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
    // Store the stdin and stdout file descriptors
    int stdin_fd = dup(STDIN_FILENO);
    int stdout_fd = dup(STDOUT_FILENO);

    // Dynamically allocate memory for line
    char* line = malloc(1024 * sizeof(char));

    while (1) {
        command_t* commands;
        int pipes_pool[64][2];
        int* pid_pool;
        int command_count = 1;
        size_t line_size = 0;

        char* token = malloc(32 * sizeof(char));

        // Reset the stdin and stdout file descriptors for each new iteration
        dup2(stdin_fd, STDIN_FILENO);
        dup2(stdout_fd, STDOUT_FILENO);

        // Print the prompt to stdout
        printf("mybash$ ");
        fflush(stdout);

        // Read the line from stdin
        getline(&line, &line_size, stdin);

        // If the line is empty then continue
        if (strcmp(line, "\n") == 0) {
            continue;
        }

        // if the line says 'exit' then exit the shell
        if (strcmp(line, "exit\n") == 0) {
            exit(0);
        }

        // Remove the trailing newline character
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        // Count the number of commands in the line
        for (int i = 0; i < strlen(line) - 1; i++) {
            if (line[i] == '|' || line[i] == '&' || line[i] == ';') {
                if (line[i + 1] == line[i]) {
                    i++;
                }
                command_count++;
            }
        }

        // Dynamically allocate memory for commands and operations
        commands = malloc(command_count * sizeof(command_t));
        // Initialize the commands and operations
        for (int i = 0; i < command_count; i++) {
            INIT_COMMAND(commands[i]);
        }

        // Process the line and extract separate commands
        // and store the command in commands array
        process_line(line, commands);

        // Dynamically allocate memory for pid pool
        pid_pool = malloc(command_count * sizeof(int));

        // Process each commands and create pool of pipes
        for (int i = 0; i < command_count; i++) {
            command_t* cmd = &commands[i];
            process_command(cmd);

            // Create pipes for each command
            if (pipe(pipes_pool[i]) < 0) {
                printf("pipe failed\n");
                exit(1);
            }
        }

        int last_status[1] = {0}; // 0 = fail and 1 = success

        // Iterate over the commands and execute them
        for (int i = 0; i < command_count; i++) {

            if (i != 0) {
                if (commands[i - 1].c_operation == LOGICAL_AND) {

                    if (last_status[0] != 0) {
                        // If command fails and next operation is OR then Skip
                        int k = i + 1;
                        for (; k < command_count; k++) {
                            if (commands[k].c_operation == LOGICAL_AND) {
                                continue;
                            } else {
                                break;
                            }
                        }
                        i = k;
                    }
                } else if (commands[i - 1].c_operation == LOGICAL_OR) {
                    if (last_status[0] == 0) {
                        int k = i + 1;
                        for (; k < command_count; k++) {
                            if (commands[k].c_operation == LOGICAL_OR) {
                                continue;
                            } else {
                                break;
                            }
                        }
                        i = k;
                    }
                }
            }
            last_status[0] = 1;

            // Fork the process and execute individual command
            pid_pool[i] = fork();
            if (pid_pool[i] < 0) {
                perror("fork failed\n");
            } else if (pid_pool[i] == 0) {
                int fd = -1;

                // Change stdin for all the commands except the first one
                // and commands with no redirection
                if (i != 0
                    || commands[i].c_redirection_operation == NO_SUB_OP) {
                    dup2(pipes_pool[i - 1][0], STDIN_FILENO);
                    close(pipes_pool[i - 1][0]);
                }

                // Change stdout for all the commands except the last one,
                // commands with change and commands with no redirection
                if (i != command_count - 1 && commands[i].c_operation != CHAIN
                    && commands[i].c_operation != LOGICAL_AND
                    && commands[i].c_operation != LOGICAL_OR
                    && commands[i].c_redirection_operation == NO_SUB_OP) {
                    dup2(pipes_pool[i][1], STDOUT_FILENO);
                    close(pipes_pool[i][1]);
                }

                //  Handle the redirection operations
                if (commands[i].c_redirection_operation == REDIRECT_OUT) {

                    // Set STDOUT to the output file
                    fd = open(commands[i].c_redirection_file,
                              O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                } else if (commands[i].c_redirection_operation
                           == REDIRECT_APPEND) {

                    // Set STDOUT to the output file with append
                    fd = open(commands[i].c_redirection_file,
                              O_WRONLY | O_CREAT | O_APPEND, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                } else if (commands[i].c_redirection_operation == REDIRECT_IN) {

                    // Set STDIN to the input file
                    fd = open(commands[i].c_redirection_file, O_RDONLY, 0644);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                // Close all unused pipes
                for (int j = 0; j < command_count; j++) {
                    close(pipes_pool[j][0]);
                    close(pipes_pool[j][1]);
                }

                if (BACKGROUND) {
                    dup2(open("/dev/null", O_WRONLY), STDOUT_FILENO);
                    setpgid(0, 0);
                    BACKGROUND = 0;
                }

                // Execute the command
                if (execvp(commands[i].c_name, commands[i].c_args) < 0) {
                    printf("%s: command not found\n", commands[i].c_name);
                    last_status[0] = 0;
                    exit(1);
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
                BACKGROUND = 0;
            }
        }
    }
    return 0;
}