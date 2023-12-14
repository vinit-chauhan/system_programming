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

// server and mirror configurations
#define IS_MIRROR     0
#define PORT          5000
#define MIRROR_ADDR   "127.0.0.1"
#define MIRROR_PORT   5001

// buffer size configurations
#define MAX_LINE      1024
#define SEND_MAX_LINE 10 * 1024
#define MAX_CLIENTS   100

// global variables
int server_pid;
int socket_fd;
int c_socket_fd[MAX_CLIENTS];
int* c_pid_pool;

void
terminate_fork(int signum) {
    exit(0);
}

// termination function
void
terminate(int signum) {
    printf("Terminating Server...\n");

    // Close all client connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (c_socket_fd[i] == 0) {
            continue;
        }
        close(c_socket_fd[i]);
        kill(c_pid_pool[i], SIGTERM);
    }

    // Close server socket
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}

// client request process id pool
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

// separate space delimited tokens from the given string
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

// increase date by given increment
char*
increase_date(char* val, int increment) {
    char* date = malloc(strlen(val) * sizeof(char));
    strcpy(date, val);

    // last day of each month
    int last_day[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int year = atoi(strtok(date, "-"));
    int month = atoi(strtok(NULL, "-"));
    int day = atoi(strtok(NULL, ""));

    // reset date buffer
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

    // create date string from the incremented values
    sprintf(date, "%d-%d-%d", year, month, day);

    // return the date string
    return date;
}

// process the command and create the command to be executed
void
process_command(char* cmd, char* command) {
    char* tokens[4] = {NULL};
    tokenizer(cmd, tokens);

    // create command to be executed based on the command type
    // name format of server-side tarball: <pid>.tar.gz
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

// execute the given command and don't store the output
void
execute_command_no_out(char* command) {
    // save stderr
    int stderr = dup(2);
    // open /dev/null
    int null_fd = open("/dev/null", O_WRONLY);

    // redirect stderr to /dev/null
    dup2(null_fd, 2);

    // execute the command
    FILE* op = popen(command, "r");
    if (op == NULL) {
        perror("popen");
        exit(1);
    }

    // restore stderr
    dup2(stderr, 2);

    // close the file pointer
    pclose(op);
}

// execute the given command and store the output in the given buffer
void
execute_command(char* command, char* output) {
    // save stderr
    int stderr = dup(2);
    // open /dev/null
    int null_fd = open("/dev/null", O_WRONLY);

    // redirect stderr to /dev/null
    dup2(null_fd, 2);

    // execute the command
    FILE* op = popen(command, "r");
    if (op == NULL) {
        perror("popen");
        exit(1);
    }

    // restore stderr
    dup2(stderr, 2);

    // read the output of the command
    fgets(output, MAX_LINE, op);

    // remove the trailing newline character
    if (output[strlen(output) - 1] == '\n') {
        output[strlen(output) - 1] = '\0';
    }

    // close the file pointer
    pclose(op);
}

void
process_output(int out_socket, char* command, char* output_buffer) {

    // if the output is empty, send error message
    if (output_buffer[0] == '\0') {
        int total_sent_bytes = send(out_socket, "__err_nofile", 13, 0);
        printf("Sent %d bytes\n", total_sent_bytes);
        return;
    }

    // if the command is getfn, send the output as it is
    if (strcmp(command, "getfn") == 0) {
        int total_sent_bytes =
            send(out_socket, output_buffer, strlen(output_buffer), 0);
        printf("Sent %d bytes\n", total_sent_bytes);

    } else { // else, send the tarball
        int total_sent_bytes = 0;

        // create tarball name buffer
        char* file_name = malloc(16 * sizeof(char));
        memset(file_name, 0, 16);

        // create tarball name : <pid>.tar.gz and open the file
        sprintf(file_name, "%d.tar.gz", getpid());
        int tar_fd = open(file_name, O_RDONLY);

        // send the file size
        // create read buffer
        char* buff = malloc(SEND_MAX_LINE * sizeof(char));
        while (1) {
            int read_bytes = read(tar_fd, buff, SEND_MAX_LINE);
            if (read_bytes < 1) {
                break;
            }

            // send the read buffer to client
            int send_bytes = send(out_socket, buff, read_bytes, 0);

            // increment total sent bytes
            total_sent_bytes += send_bytes;
        }

        // close the file and free the buffer
        free(buff);
        close(tar_fd);

        printf("Sent %d bytes\n", total_sent_bytes);

        // remove the tarball once its sent
        if (remove(file_name) != 0) {
            perror("remove");
        }
    }
}

// handle client request
void
pclientrequest(int l_socket_fd) {
    int pid = -1;
    // fork a child process to handle the request
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) { // child process

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
            } else if (recv_byte == 0) { // if client disconnected
                printf("Client disconnected\n");
                break;
            } else { // if command received
                printf("Received %d bytes\n", recv_byte);
            }

            // remove the trailing newline character
            read_buff[recv_byte] = '\0';

            // check if quit command is received
            if (strcmp(read_buff, "quitc") == 0) {
                printf("Client disconnected\n");
                close(l_socket_fd);
                exit(0);
            }

            // create command buffer and initialize with 0
            char* exec_cmd = malloc(MAX_LINE * sizeof(char));
            memset(exec_cmd, 0, MAX_LINE);

            // process the command
            process_command(read_buff, exec_cmd);

            // create write buffer and initialize with 0
            memset(write_buff, 0, MAX_LINE);

            // execute the command
            execute_command(exec_cmd, write_buff);
            free(exec_cmd);

            // create buffer and initialize with 0
            char* cmd = malloc(16 * sizeof(char));
            memset(cmd, 0, 16);

            // get the command type from the read buffer
            strcpy(cmd, strtok(read_buff, " "));

            // process the output
            process_output(l_socket_fd, cmd, write_buff);
            free(write_buff);
            free(read_buff);
        }
    } else { // parent process

        // add the child process id to the pool
        set_pid(pid);
    }
}

int
main(int argc, char* argv[]) {
    struct sockaddr_in servAdd;

    // set signal handlers
    signal(SIGINT, terminate);

    // initialize client pid pool
    c_pid_pool = malloc(MAX_CLIENTS * sizeof(int));

    server_pid = getpid();

    // request counter
    int req_count = 0;
    // redirect flag
    int redirect = 0;

    printf("Server starting...\n");

    // create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    // set socket options
    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdd.sin_port = htons((uint16_t)IS_MIRROR ? MIRROR_PORT : PORT);

    // bind socket to address
    if (bind(socket_fd, (struct sockaddr*)&servAdd, sizeof(servAdd)) < 0) {
        perror("bind");
        return 1;
    }

    // listen for connections
    if (listen(socket_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server listening...\n");

    while (1) {
        // accept connection from client
        if ((c_socket_fd[req_count++] =
                 accept(socket_fd, (struct sockaddr*)NULL, NULL))
            == -1) {
            perror("accept");
            break;
        } else {
            printf("Connection accepted from client.\n");
        }

        // handle request based on if the server is mirror or not
        if (IS_MIRROR) {
            // if mirror, server all the requests
            pclientrequest(c_socket_fd[req_count - 1]);

        } else {
            // if not mirror, redirect request 4-8 and every 2nd request after that.
            if (req_count <= 4) {
                redirect = 0;
            } else if (req_count <= 8) {
                redirect = 1;
            } else {
                redirect = (req_count % 2 == 0);
            }

            if (!redirect) {
                // if the request is not to be redirected, serve the request
                send(c_socket_fd[req_count - 1], "ok\n", 3, 0);
                pclientrequest(c_socket_fd[req_count - 1]);

            } else {
                // redirect to another server
                char* redirect_msg = malloc(32 * sizeof(char));
                memset(redirect_msg, 0, 32);

                // create redirect message format: <mirror_address> <mirror_port>
                sprintf(redirect_msg, "%s %d\n", MIRROR_ADDR, MIRROR_PORT);

                // send redirect message to client
                int total_sent_bytes =
                    send(c_socket_fd[req_count - 1], redirect_msg,
                         strlen(redirect_msg), 0);
                printf("Redirection info sent.\n");

                // close client connection
                close(c_socket_fd[req_count]);
            }
        }
    }

    return 0;
}
