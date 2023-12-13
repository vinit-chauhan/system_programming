#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT     5000
#define MAX_LINE 1024

int socket_fd;

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

        // check if the second argument is a number
        for (int i = 0; i < strlen(tokens[1]); i++) {
            if (tokens[1][i] < '0' || tokens[1][i] > '9') {
                printf("Error: Argument 2 Not a Number : Usage: getfz size1 "
                       "size2 \n");
                return 1;
            }
        }

        // check if the third argument is a number
        for (int i = 0; i < strlen(tokens[2]); i++) {
            if (tokens[2][i] < '0' || tokens[2][i] > '9') {
                printf("Error: Argument 3 Not a Number: Usage: getfz size1 "
                       "size2 \n");
                return 1;
            }
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

    int rcv = -1;

    signal(SIGINT, func_term);

    // create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    // Initialize socket structure
    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAdd.sin_port = htons((uint16_t)5000);

    // connect the client socket to server socket
    if (connect(socket_fd, (struct sockaddr*)&servAdd, sizeof(servAdd)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        printf("connected to the server..\n");
    }

    // create f23project directory in home dir
    if (system("test -d ~/f23project") != 0) {
        system("mkdir ~/f23project");
    }

    // go into client cli loop
    while (1) {
        char* write_buff = malloc(MAX_LINE * sizeof(char));
        char* read_buff = malloc(MAX_LINE * sizeof(char));
        printf("Client$ ");
        fflush(stdout);
        fgets(write_buff, MAX_LINE, stdin);
        write_buff[strlen(write_buff) - 1] = '\0';

        if (validate_commands(write_buff) == 1) {
            printf("Invalid command\n");
            continue;
        }

        // send command to server
        int n = 0;
        if ((n = send(socket_fd, write_buff, strlen(write_buff), 0)) < 0) {
            perror("send");
            exit(1);
        } else {
            printf("Sent %d bytes\n", n);
        }

        char* cmd = strtok(write_buff, " ");

        // wait for server to send response
        if ((rcv = recv(socket_fd, read_buff, MAX_LINE, 0)) < 0) {
            perror("recv");
            exit(1);
        } else if (rcv == 0) {
            printf("Server disconnected\n");
            break;
        } else {
            printf("%s\n", read_buff);
        }

        free(write_buff);
        free(read_buff);
    }

    return 0;
}
