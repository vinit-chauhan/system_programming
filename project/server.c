#include <netinet/in.h> //structure for storing address information
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <unistd.h> //for read and write
#include <wait.h>

#define PORT        5000
#define MAX_LINE    1024
#define MAX_CLIENTS 5

int socket_fd;

// request
// -- create fork
// -- read from socket
// -- parse request
// -- execute request
// -- write to socket
// -- close socket

void
pclientrequest(int l_socket_fd) {
    int pid;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        while (1) {
            char* buffer = malloc(MAX_LINE * sizeof(char));
            read(l_socket_fd, buffer, MAX_LINE);

            printf("Client: %s, %d\n", buffer, strcmp(buffer, "quitc"));

            if (strcmp(buffer, "quitc") == 0) {
                printf("Client %d disconnected\n", l_socket_fd);
                // close(l_socket_fd);
                shutdown(l_socket_fd, SHUT_RDWR);
                break;
            }

            printf("After check Client: %s\n", buffer);
        }
    }
}

void
terminate(int signum) {
    printf("Terminating server...\n");
    shutdown(socket_fd, SHUT_RDWR);
    exit(0);
}

int
main(int argc, char* argv[]) {
    struct sockaddr_in servAdd;
    signal(SIGINT, terminate);

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

    while (1) {
        struct sockaddr_in clientAdd;
        int l_socket_fd;
        if ((l_socket_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL))
            == -1) {
            perror("accept");
            exit(1);
        } else {
            printf("Connection accepted from client: %d\n", l_socket_fd);
        }

        pclientrequest(l_socket_fd);
    }

    return 0;
}
