#include <fcntl.h>
#include <netinet/in.h> //structure for storing address information
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define PORT        5000
#define MAX_LINE    1024
#define MAX_CLIENTS 5

int quit = 0;

int server_pid;
int socket_fd;
int c_socket_fd[MAX_CLIENTS];
int* c_pid_pool;
int num_clients = 1;

void
terminate_fork(int signum) {
    printf("Terminating Active Connection...\n");
    quit = 1;
    exit(0);
}

void
terminate(int signum) {
    printf("Terminating Server...\n");
    quit = 1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        printf("%d\n", c_pid_pool[i]);
    }

    // Close all client connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (c_socket_fd[i] == 0) {
            continue;
        }
        close(c_socket_fd[i]);
        sleep(1);
        kill(c_pid_pool[i], SIGTERM);
    }

    // Close server socket
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}

int
set_pid(int pid) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (c_pid_pool[i] == 0) {
            c_pid_pool[i] = pid;
            return 0;
        }
    }
    return 1;
}

int
unset_pid(int pid) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (c_pid_pool[i] == pid) {
            c_pid_pool[i] = 0;
            return 0;
        }
    }
    num_clients--;
    return 1;
}

char**
tokenizer(char* str, char** tokens) {
    char* cmd_copy = malloc(strlen(str) * sizeof(char));
    strcpy(cmd_copy, str);

    char* token = strtok(cmd_copy, " ");
    int count = 0;

    while (token != NULL) {
        tokens[count++] = token;
        token = strtok(NULL, " ");
    }

    return tokens;
}

void
process_command(char* cmd, char* command) {
    char* tokens[4];
    tokenizer(cmd, tokens);

    if (strcmp(tokens[0], "getfn") == 0) {
        sprintf(command,
                "(ls -la \"$(find ~ -name '%s' -type f | "
                "head -1)\" || ls -la ~ | grep -x \"%s\") | awk "
                "'{print \"File Name: "
                "\"$9\"\\tSize: "
                "\"$5\"\\tDate: \"$6\" \"$7\"\\tPermissions: \"$1}'",
                tokens[1], tokens[1]);

    } else if (strcmp(tokens[0], "getfz") == 0) {
        // update the temo folder to home dir.
        sprintf(command,
                "find ~/test/ -type f -size +%sc -a -size -%sc -exec tar "
                "czvf temp.tar.gz {} +",
                tokens[1], tokens[2]);
    } else if (strcmp(tokens[0], "getft") == 0) {
    } else if (strcmp(tokens[0], "getfdb") == 0) {
    } else if (strcmp(tokens[0], "getfda") == 0) {
    }
}

void
execute_command(char* command, char* output) {
    int stderr = dup(2);
    int null_fd = open("/dev/null", O_WRONLY);
    dup2(null_fd, 2);
    FILE* op = popen(command, "r");
    if (op == NULL) {
        perror("popen");
        exit(1);
    }

    fgets(output, MAX_LINE, op);

    if (output[strlen(output) - 1] == '\n') {
        output[strlen(output) - 1] = '\0';
    }

    printf("Output: %s\n", output);
    pclose(op);
}

void
process_output(int out_socket, char* command, char* output_buffer) {

    printf("Command: %s\n", command);

    if (output_buffer[0] == '\0') {
        int snd = send(out_socket, "No file found", 13, 0);
    }

    if (strcmp(command, "getfn") == 0) {
        int snd = send(out_socket, output_buffer, strlen(output_buffer), 0);
    } else {
        // send the tar file
        // delete the tar file
    }
}

void
pclientrequest(int l_socket_fd) {
    int pid = -1;
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        char *write_buff, *read_buff;

        signal(SIGTERM, terminate_fork);

        while (1) {
            read_buff = malloc(MAX_LINE * sizeof(char));
            memset(read_buff, 0, MAX_LINE);

            // wait for client to send command
            int recv_byte = -1;
            if ((recv_byte = recv(l_socket_fd, read_buff, MAX_LINE, 0)) < 0) {
                perror("recv");
                exit(1);
            } else if (recv_byte == 0) {
                printf("Client disconnected\n");
                unset_pid(getpid());
                break;
            } else {
                printf("Received %d bytes: %s\n", recv_byte, read_buff);
            }

            read_buff[recv_byte] = '\0';

            if (strcmp(read_buff, "quitc") == 0) {
                printf("Client %d disconnected\n", l_socket_fd);
                unset_pid(getpid());
                kill(server_pid, SIGQUIT);
                kill(getpid(), SIGTERM);
                break;
            }

            char* cmd = malloc(MAX_LINE * sizeof(char));
            memset(cmd, 0, MAX_LINE);

            process_command(read_buff, cmd);

            // create write buffer and initialize with 0
            write_buff = malloc(MAX_LINE * sizeof(char));
            memset(write_buff, 0, MAX_LINE);

            execute_command(cmd, write_buff);
            free(cmd);

            char* token = malloc(16 * sizeof(char));
            memset(token, 0, 16);

            strcpy(token, strtok(read_buff, " "));

            process_output(l_socket_fd, token, write_buff);
            free(write_buff);
            free(read_buff);
        }
    } else {
        set_pid(pid);
    }
}

int
main(int argc, char* argv[]) {
    struct sockaddr_in servAdd;
    signal(SIGINT, terminate);
    signal(SIGQUIT, terminate);

    server_pid = getpid();
    c_pid_pool = malloc(MAX_CLIENTS * sizeof(int));

    printf("Server starting...\n");

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdd.sin_port = htons((uint16_t)PORT);

    if (bind(socket_fd, (struct sockaddr*)&servAdd, sizeof(servAdd)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(socket_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server listening...\n");

    int i = 0;
    while (!quit) {
        // will have to handle i, if client disconnects, the next client will take up empty slot.
        if ((c_socket_fd[i++] = accept(socket_fd, (struct sockaddr*)NULL, NULL))
            == -1) {
            perror("accept");
            break;
        } else {
            printf("Connection accepted from client: %d\n", num_clients++);
        }

        pclientrequest(c_socket_fd[i - 1]);

        sleep(10);
    }

    return 0;
}
