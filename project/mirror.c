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

#define IS_MIRROR     1
#define PORT          5000
#define MIRROR_ADDR   "127.0.0.1"
#define MIRROR_PORT   5001

#define MAX_LINE      1024
#define SEND_MAX_LINE 10 * 1024
#define MAX_CLIENTS   100

int server_pid;
int socket_fd;
int c_socket_fd[MAX_CLIENTS];
int* c_pid_pool;

void
terminate_fork(int signum) {
    exit(0);
}

void
terminate(int signum) {
    printf("Terminating Server...\n");

    // Close all client connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (c_socket_fd[i] == 0) {
            continue;
        }
        close(c_socket_fd[i]);
        // sleep(1);
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

char*
increase_date(char* val, int increment) {
    char* date = malloc(strlen(val) * sizeof(char));
    strcpy(date, val);

    int last_day[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int year = atoi(strtok(date, "-"));
    int month = atoi(strtok(NULL, "-"));
    int day = atoi(strtok(NULL, ""));

    memset(date, 0, strlen(date));

    day += increment;

    if (day > last_day[month - 1]) {
        day = 1;
        month++;
    }

    if (month > 12) {
        month = 1;
        year++;
    }

    sprintf(date, "%d-%d-%d", year, month, day);

    return date;
}

void
process_command(char* cmd, char* command) {
    char* tokens[4] = {NULL};
    tokenizer(cmd, tokens);

    printf("pid: %d\n", getpid());

    if (strcmp(tokens[0], "getfn") == 0) {
        sprintf(command,
                "(ls -la \"$(find ~ -name '%s' -type f | "
                "head -1)\" || ls -la ~ | grep -x \"%s\") | awk "
                "'{print \"File Name: "
                "\"$9\"\\tSize: "
                "\"$5\"\\tDate: \"$6\" \"$7\"\\tPermissions: \"$1}'",
                tokens[1], tokens[1]);

    } else if (strcmp(tokens[0], "getfz") == 0) {
        sprintf(command,
                "find ~/ -type f -size +%dc -a -size -%dc -exec tar "
                "czvf %d.tar.gz {} +",
                atoi(tokens[1]) - 1, atoi(tokens[2]) + 1, getpid());
    } else if (strcmp(tokens[0], "getft") == 0) {
        char* temp = malloc(64 * sizeof(char));
        memset(temp, 0, 64);

        for (int i = 1, cur = 0; i < 4; i++) {
            if (tokens[i] != NULL) {
                char* temp2;
                if (i == 3 || tokens[i + 1] == NULL) {
                    temp2 = malloc(16 * sizeof(char));
                    memset(temp2, 0, 16);
                    sprintf(temp2, "-name '*.%s'", tokens[i]);
                } else {
                    temp2 = malloc(16 * sizeof(char));
                    memset(temp2, 0, 16);
                    sprintf(temp2, "-name '*.%s' -o ", tokens[i]);
                }
                strcat(temp, temp2);
            }
        }

        sprintf(command,
                "find ~/ -type f \\( %s \\) -exec tar czvf %d.tar.gz {} +",
                temp, getpid());
    } else if (strcmp(tokens[0], "getfdb") == 0) {
        char* new_date = increase_date(tokens[1], 1);
        sprintf(command,
                "find ~/test -type f ! -newermt \"%s\" ! -newermt "
                "\"%s\" -exec tar czvf %d.tar.gz {} +",
                tokens[1], new_date, getpid());
    } else if (strcmp(tokens[0], "getfda") == 0) {
        sprintf(command,
                "find ~/test -type f  -newermt \"%s\" -exec tar czvf "
                "%d.tar.gz {} +",
                tokens[1], getpid());
    }
}

void
execute_command_no_out(char* command) {
    int stderr = dup(2);
    int null_fd = open("/dev/null", O_WRONLY);
    dup2(null_fd, 2);
    FILE* op = popen(command, "r");
    if (op == NULL) {
        perror("popen");
        exit(1);
    }

    dup2(stderr, 2);
    pclose(op);
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

    dup2(stderr, 2);

    fgets(output, MAX_LINE, op);

    if (output[strlen(output) - 1] == '\n') {
        output[strlen(output) - 1] = '\0';
    }

    pclose(op);
}

void
process_output(int out_socket, char* command, char* output_buffer) {

    if (output_buffer[0] == '\0') {
        int total_sent_bytes = send(out_socket, "__err_nofile", 13, 0);
        printf("Sent %d bytes\n", total_sent_bytes);
        return;
    }

    if (strcmp(command, "getfn") == 0) {
        int total_sent_bytes =
            send(out_socket, output_buffer, strlen(output_buffer), 0);
        printf("Sent %d bytes\n", total_sent_bytes);

    } else {
        char* file_name = malloc(16 * sizeof(char));
        memset(file_name, 0, 16);
        sprintf(file_name, "%d.tar.gz", getpid());

        int tar_fd = open(file_name, O_RDONLY);
        int total_sent_bytes = 0;
        // send the file size
        while (1) {
            char* buff = malloc(SEND_MAX_LINE * sizeof(char));
            int read_bytes = read(tar_fd, buff, SEND_MAX_LINE);
            if (read_bytes < 1) {
                break;
            }
            int send_bytes = send(out_socket, buff, read_bytes, 0);
            total_sent_bytes += send_bytes;
        }

        close(tar_fd);

        printf("Sent %d bytes\n", total_sent_bytes);

        if (remove(file_name) == 0) {
            perror("remove");
        }
    }
}

void
pclientrequest(int l_socket_fd) {
    int pid = -1;
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        char* write_buff = malloc(MAX_LINE * sizeof(char));
        char* read_buff = malloc(MAX_LINE * sizeof(char));

        // set signal handlers
        signal(SIGTERM, terminate_fork);
        signal(SIGINT, SIG_DFL);

        while (1) {
            // create a read buffer and initialize with 0
            memset(read_buff, 0, MAX_LINE);

            // wait for client to send command
            int recv_byte = -1;
            if ((recv_byte = recv(l_socket_fd, read_buff, MAX_LINE, 0)) < 0) {
                perror("recv");
                exit(1);
            } else if (recv_byte == 0) {
                printf("Client disconnected\n");
                break;
            } else {
                printf("Received %d bytes\n", recv_byte);
            }

            read_buff[recv_byte] = '\0';

            if (strcmp(read_buff, "quitc") == 0) {
                printf("Client disconnected\n");
                exit(0);
            }

            char* cmd = malloc(MAX_LINE * sizeof(char));
            memset(cmd, 0, MAX_LINE);

            process_command(read_buff, cmd);

            // create write buffer and initialize with 0
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

    server_pid = getpid();
    c_pid_pool = malloc(MAX_CLIENTS * sizeof(int));

    int req_count = 0;
    int redirect = 0;

    printf("Server starting...\n");

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdd.sin_port = htons((uint16_t)IS_MIRROR ? MIRROR_PORT : PORT);

    if (bind(socket_fd, (struct sockaddr*)&servAdd, sizeof(servAdd)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(socket_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server listening...\n");

    while (1) {
        sleep(1);
        // will have to handle i, if client disconnects, the next client will take up empty slot.
        if ((c_socket_fd[req_count++] =
                 accept(socket_fd, (struct sockaddr*)NULL, NULL))
            == -1) {
            perror("accept");
            break;
        } else {
            printf("Connection accepted from client.\n");
        }

        if (IS_MIRROR) {
            pclientrequest(c_socket_fd[req_count - 1]);
        } else {

            if (req_count <= 4) {
                redirect = 0;
            } else if (req_count <= 8) {
                redirect = 1;
            } else {
                redirect = (req_count % 2 == 0);
            }

            printf("Redirect: %d\n", redirect);
            printf("Req count: %d\n", req_count);

            if (!redirect) {
                send(c_socket_fd[req_count - 1], "ok\n", 3, 0);
                pclientrequest(c_socket_fd[req_count - 1]);
            } else {
                // redirect to another server
                char* redirect_msg = malloc(32 * sizeof(char));
                memset(redirect_msg, 0, 32);
                sprintf(redirect_msg, "%s %d\n", MIRROR_ADDR, MIRROR_PORT);

                int total_sent_bytes =
                    send(c_socket_fd[req_count - 1], redirect_msg,
                         strlen(redirect_msg), 0);
                printf("Redirection msg sent.\n");
                close(c_socket_fd[req_count]);
            }
        }
    }

    return 0;
}
