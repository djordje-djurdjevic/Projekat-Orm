//peers

#include <stdio.h>          //printf
#include <stdlib.h>         //exit codes
#include <string.h>         //strlen
#include <sys/socket.h>     //socket
#include <arpa/inet.h>      //inet_addr
#include <fcntl.h>          //for open
#include <unistd.h>         //for close

#define IP_ADDRESS "127.0.0.1"
#define PORT 12345
#define DEFAULT_BUFLEN 512

int main() {

    int client_socket_fd;
    struct sockaddr_in server_address;
    char message[DEFAULT_BUFLEN] = "ZEMOOOOOOO";

    //create socket;
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0 );
    if (client_socket_fd == -1) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }
    else {
        printf("Socket successfully created\n\n");
    }

    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    server_address.sin_port        = htons(PORT); 

    if (connect(client_socket_fd, (struct sockaddr *)&server_address , 
    sizeof(server_address)) < 0 ) {
        perror("Failed to connect");
        return EXIT_FAILURE;
    }
    else {
           printf("Successfully connected to server [%s:%hu]\n\n",
            inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));  
    }

    if(send(client_socket_fd, message, strlen(message), 0 ) < 0 ) {
        printf("Message sending failed");
    }
    else {
        printf("Message '%s' successfully sent to the server\n\n", message);
    }

    close(client_socket_fd);
    printf("Socket closed\n\n");

    return 0;
}
