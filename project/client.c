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
main(int argc, char* argv[]) {
    struct sockaddr_in servAdd;

    signal(SIGINT, func_term);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAdd.sin_port = htons((uint16_t)5000);

    if (connect(socket_fd, (struct sockaddr*)&servAdd, sizeof(servAdd)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        printf("connected to the server..\n");
    }

    char* write_buff = malloc(MAX_LINE * sizeof(char));
    char* read_buff = malloc(MAX_LINE * sizeof(char));

    while (1) {
        printf("Client$ ");
        fflush(stdout);
        fgets(write_buff, MAX_LINE, stdin);
        printf("Client: %s\n", write_buff);
        write(socket_fd, write_buff, strlen(write_buff) - 1);
    }

    return 0;
}
