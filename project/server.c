#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> //for socket APIs
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> //for read and write

int
main(int argc, char* argv[]) {
    struct sockaddr_in servAdd;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdd.sin_port = htons((uint16_t)5000);

    if (bind(socket_fd, (struct sockaddr*)&servAdd, sizeof(servAdd)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(socket_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    char* r_buffer = malloc(1024 * sizeof(char));
    char* w_buffer = malloc(1024 * sizeof(char));

    while (1) {
        int l_socket_fd;
        if ((l_socket_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL))
            == -1) {
            perror("accept");
            exit(1);
        }

        w_buffer =
            "Test Message from the server to the client: How are you doing?";

        write(l_socket_fd, w_buffer, strlen(w_buffer) + 1);

        read(l_socket_fd, r_buffer, 1024);
        printf("Client: %s\n", r_buffer);

        close(l_socket_fd);
    }

    return 0;
}
