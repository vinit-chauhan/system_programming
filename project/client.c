#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT          5000
#define MAX_BUFF_SIZE 1024

int socket_fd;
char* tar_path;

// signal handler for SIGINT
void
func_term(int signum) {
    printf("Terminating client...\n");
    close(socket_fd);
    exit(0);
}

int
validate_commands(char* cmd) {
    char* cmd_copy = malloc(strlen(cmd) * sizeof(char));
    char* tokens[4];
    strcpy(cmd_copy, cmd);
    char* token = strtok(cmd_copy, " ");
    int count = 0;

    while (token != NULL) {
        tokens[count++] = token;
        token = strtok(NULL, " ");
    }

    if (count < 2 && strcmp(tokens[0], "quitc") != 0) {
        printf("Invalid command\n");
        return 1;
    }

    if (strcmp(tokens[0], "getfn") == 0) {
        if (count != 2) {
            printf("Invalid command\n");
            return 1;
        }
    } else if (strcmp(tokens[0], "getfz") == 0) {
        printf("count: %d\n", count);
        // check if there are 3 arguments
        if (count != 3) {
            printf("Error: Invalid format: Usage: getfz size1 size2 \n");
            return 1;
        }

        int size1 = atoi(tokens[1]);
        int size2 = atoi(tokens[2]);

        if ((size1 <= 0 && tokens[1][0] != '0')
            || (size2 <= 0 && tokens[2][0] != '0')) {
            printf("Error: Invalid format: Usage: getfz size1 size2 \n");
            return 1;
        }

        if (size1 > size2) {
            printf("Error: size1 should be less than size2: Usage: "
                   "getfz size1 size2 \n");
            return 1;
        }

    } else if (strcmp(tokens[0], "getft") == 0) {
        if (count > 4) {
            printf(
                "Error: Invalid format: Usage: getft <extension list> // upto "
                "3\n");
            return 1;
        }

        // check if all the args are different
        for (int i = 1; i < count; i++) {
            for (int j = i + 1; j < count; j++) {
                if (strcmp(tokens[i], tokens[j]) == 0) {
                    printf("Error: File Types are not unique\n");
                    return 1;
                }
            }
        }

    } else if (strcmp(tokens[0], "getfdb") == 0
               || strcmp(tokens[0], "getfda") == 0) {
        // check if there is 1 argument and it is a date.
        if (count != 2) {
            printf("Error: Invalid format: Usage: %s date \n", tokens[0]);
            return 1;
        }
        char* date = tokens[1];
        if (strlen(date) != 10) {
            printf("Error: Invalid date format. Expected format: yyyy-mm-dd\n");
            return 1;
        }

        for (int i = 0; i < 10; i++) {
            if (i == 4 || i == 7) {
                if (date[i] != '-') {
                    printf("Error: Invalid date format. Expected format: "
                           "yyyy-mm-dd\n");
                    return 1;
                }
            } else {
                if (date[i] < '0' || date[i] > '9') {
                    printf("Error: Invalid date format. Expected format: "
                           "yyyy-mm-dd\n");
                    return 1;
                }
            }
        }

    } else if (strcmp(tokens[0], "quitc") == 0) {
        return 0;
    } else {
        printf("Invalid command\n");
        return 1;
    }

    return 0;
}

int
main(int argc, char* argv[]) {
    struct sockaddr_in servAdd;
    tar_path = malloc(128 * sizeof(char));
    tar_path = getenv("HOME");

    int rcv = -1;

    if (argc != 3) {
        printf("Usage: %s <server IP> <server port>\n", argv[0]);
        return 1;
    }

    char* SRV_ADDR = malloc(16 * sizeof(char));
    strcpy(SRV_ADDR, argv[1]);
    int SRV_PORT = atoi(argv[2]);

    signal(SIGINT, func_term);

    // create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    // Initialize socket structure
    servAdd.sin_family = AF_INET;

    //  TODO: Fetch IP and port from user.
    servAdd.sin_addr.s_addr = inet_addr(SRV_ADDR);
    servAdd.sin_port = htons((uint16_t)SRV_PORT);

    // connect the client socket to server socket
    if (connect(socket_fd, (struct sockaddr*)&servAdd, sizeof(servAdd)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }

    // create f23project directory in home dir
    if (system("test -d ~/f23project") != 0) {
        system("mkdir ~/f23project");
    }

    // set tar destination path
    strcat(tar_path, "/f23project/temp.tar.gz");

    // receive server response on initial connection
    char* buff = malloc(128 * sizeof(char));
    int n = recv(socket_fd, buff, 128, 0);
    buff[n - 1] = '\0';

    // check if server responded with ok or mirror address
    if (strcmp(buff, "ok") != 0) {
        char* mirror_addr = malloc(16 * sizeof(char));
        int mirror_port = 0;
        sscanf(buff, "%s %d", mirror_addr, &mirror_port);
        printf("Redirecting to mirror...\n");
        close(socket_fd);

        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        servAdd.sin_addr.s_addr = inet_addr(mirror_addr);
        servAdd.sin_port = htons((uint16_t)mirror_port);

        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            return 1;
        }

        if (connect(socket_fd, (struct sockaddr*)&servAdd, sizeof(servAdd))
            != 0) {
            printf("connection with the mirror failed...\n");
            exit(0);
        } else {
            printf("Connected to the mirror..\n");
        }
    } else {
        printf("Connected to the server..\n");
    }

    // go into client cli loop
    while (1) {
        int n = 0; // used fro multiple counters.
        char* write_buff = malloc(MAX_BUFF_SIZE * sizeof(char));
        char* read_buff = malloc(10 * MAX_BUFF_SIZE * sizeof(char));
        printf("Client$ ");
        fflush(stdout);
        fgets(write_buff, MAX_BUFF_SIZE, stdin);
        if (write_buff[0] == '\n') {
            continue;
        }

        write_buff[strlen(write_buff) - 1] = '\0';

        if (validate_commands(write_buff) == 1) {
            continue;
        }

        // send command to server
        if ((n = send(socket_fd, write_buff, strlen(write_buff), 0)) < 0) {
            perror("send");
            exit(1);
        } else {
            printf("Sent %d bytes\n", n);
        }

        char* cmd = strtok(write_buff, " ");

        if (strcmp(cmd, "quitc") == 0) {
            printf("Client disconnected\n");
            break;
        }

        memset(read_buff, 0, 10 * MAX_BUFF_SIZE);
        if ((rcv = recv(socket_fd, read_buff, 10 * MAX_BUFF_SIZE, 0)) < 0) {
            perror("recv");
            exit(1);
        } else if (rcv == 0) {
            printf("Server disconnected\n");
            break;
        }

        if (strcmp(read_buff, "__err_nofile") == 0) {
            printf("No files found\n");
            continue;
        } else if (strcmp(cmd, "getfn") == 0) {
            printf("%s\n", read_buff);
            continue;
        } else {
            int tar_fd = open(tar_path, O_CREAT | O_WRONLY, 0644);
            if ((n = write(tar_fd, read_buff, rcv)) < 0) {
                perror("write");
                exit(1);
            }
            int total_rcv = rcv;
            while (rcv > 0 && rcv == 10 * MAX_BUFF_SIZE) {
                if ((rcv = recv(socket_fd, read_buff, 10 * MAX_BUFF_SIZE, 0))
                    < 0) {
                    perror("recv");
                    exit(1);
                }
                read_buff[rcv] = '\0';
                if ((n = write(tar_fd, read_buff, rcv)) < 0) {
                    perror("write");
                    exit(1);
                }
                total_rcv += rcv;
                // printf("Received %d bytes\n", rcv);
            }
            printf("Received %d bytes.\n", total_rcv);
            close(tar_fd);
        }
        free(write_buff);
        free(read_buff);
    }

    return 0;
}
