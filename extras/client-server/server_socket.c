#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    char buffer[1024];

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error in socket creation");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_address,
             sizeof(server_address))
        == -1) {
        perror("Error in binding");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Error in listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening...\n");

    // Accept a client connection
    if (client_socket = accept(server_socket, (struct sockaddr*)&client_address,
                               &client_address_len)
                        == -1) {
        perror("Error in accepting");
        exit(EXIT_FAILURE);
    }

    // Receive data from the client
    read(client_socket, buffer, sizeof(buffer));
    printf("Received from client: %s\n", buffer);

    // Send a response back to the client
    const char* response = "Hello from server!";
    write(client_socket, response, strlen(response));

    // Close the sockets
    close(client_socket);
    close(server_socket);

    return 0;
}
