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
#define MAX_PEERS 10
#define MAX_SEGMENTS 100

typedef struct {
    char ip[16];      // IP peer-a
    int port;         // port peer-a
} Peer;

Peer segment_owners[MAX_SEGMENTS][MAX_PEERS];
int segment_count[MAX_SEGMENTS] = {0}; // koliko peer-a ima dati segment


int main(){

    int server_socket_fd;
    int client_socket_fd;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[DEFAULT_BUFLEN];

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
    printf("Tracker server listening on port %d...\n\n", PORT);

    //accept connection
    while(1) {

        client_socket_fd = accept(server_socket_fd,(struct sockaddr *)&client_address, (socklen_t*)&len);

        if (client_socket_fd < 0 ) { perror("Accept failed"); continue;}
        printf("Peer connected: %s%d",inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        int read_size;
        while((read_size = recv(client_socket_fd, buffer, DEFAULT_BUFLEN, 0)) > 0) {
            buffer[read_size] = '\0';
            
            if (strncmp(buffer, "GET", 3) == 0) {
                int seg;
                sscanf(buffer + 4, "%d", &seg);
                if (segment_count[seg] > 0) {
                    char resp[64];
                    sprintf(resp, "%s:%d",
                            segment_owners[seg][0].ip,
                            segment_owners[seg][0].port);
                    send(client_socket_fd, resp, strlen(resp), 0);
                } else {
                    char *msg = "NO_PEER";
                    send(client_socket_fd, msg, strlen(msg), 0);
                }
            } else if (strncmp(buffer, "UPLOAD", 6) == 0) {
                int seg;
                sscanf(buffer + 7, "%d", &seg);
                int count = segment_count[seg];
                strcpy(segment_owners[seg][count].ip, inet_ntoa(client_address.sin_addr));
                segment_owners[seg][count].port = ntohs(client_address.sin_port);
                segment_count[seg]++;
                send(client_socket_fd, "OK", 2, 0);
        }
        
        printf("Peer disconnected: %s:%d\n",
        inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        close(client_socket_fd);
        }
    }


    close(server_socket_fd);
    printf("Server socket closed\n\n");

    return 0;
}   
