#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[1024];

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error in socket creation");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Use the server's IP address

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Error in connection");
        exit(EXIT_FAILURE);
    }

    // Send a message to the server
    const char* message = "Hello from client!";
    write(client_socket, message, strlen(message));

    // Receive a response from the server
    read(client_socket, buffer, sizeof(buffer));
    printf("Received from server: %s\n", buffer);

    // Close the socket
    close(client_socket);

    return 0;
}
