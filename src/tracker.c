//Server - Tracker

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#define IP_ADDRESS "127.0.0.1"
#define PORT 12345
#define DEFAULT_BUFLEN 512

int main(){

    int server_socket_fd;
    int client_socket_fd;
    int read_size;
    socklen_t len;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    char client_message[DEFAULT_BUFLEN];

    //creating the socked ipv4 tcp
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == 1) {

        perror("Socket creation failed");
        return EXIT_FAILURE;
    }
    else {
        printf("Socket successfully created\n\n");
    }

    //server_adr parameters
    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port        = htons(PORT);

    //binding to the port
    if(bind(server_socket_fd,(struct sockaddr *)&server_address ,
    sizeof(server_address)) < 0) {

        perror("Bind Failed. Error");
        return EXIT_FAILURE;
    }
    else {
        printf("Socked bound successfully to [%s:%hu]\n\n", 
        inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
    }

    //listen - max to wait before accept 5
    listen(server_socket_fd, 5);

    printf("Waiting for incoming connections...\n\n");
    len = sizeof(struct sockaddr_in);

    //accept connection
    client_socket_fd = accept(server_socket_fd,
    (struct sockaddr *)&client_address, (socklen_t*)&len);
    if (client_socket_fd < 0 ) {
        perror("Accept client failed");
        return EXIT_FAILURE;
    }
    else {
        printf("CLient [%s:%hu] connection successfully accepted\n\n",
        inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
    }

    //temporary
    while( (read_size = recv(client_socket_fd, client_message,
    DEFAULT_BUFLEN, 0 )) > 0 ) {
        client_message[read_size] = '\0';
        printf("Message received: %s\n", client_message);
        printf("Bytes received: %d\n", read_size);
    }

    if(read_size == 0) {
        printf("Client disconnected\n\n");

        close(client_socket_fd);
        printf("Client socket closed\n");
    }
    else if(read_size == -1) {
        perror("Recive client message failed");
    }

    close(server_socket_fd);
    printf("Server socket closed\n\n");

    return 0;
}   
