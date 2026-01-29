//peers

#include <stdio.h>          //printf
#include <stdlib.h>         //exit codes
#include <string.h>         //strlen
#include <sys/socket.h>     //socket
#include <arpa/inet.h>      //inet_addr
#include <fcntl.h>          //for open
#include <unistd.h>         //for close
#include <pthread.h>
#include "segment.h"


#define TRACKER_IP "127.0.0.1"
#define TRACKER_PORT 12345
#define DEFAULT_BUFLEN 512

void *peer_server(void *arg) {
    int port = *(int*)arg;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock_fd, 5);

    while (1) {
        int client_fd = accept(sock_fd, NULL, NULL);
        unsigned char buffer[512];
        FILE *fp = fopen("segments/segment_0.dat", "rb"); // primer, posalji prvi segment
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            send(client_fd, buffer, n, 0);
        }
        fclose(fp);
        close(client_fd);
    }
    close(sock_fd);
    return NULL;
}

//helper function
int connect_to(char *ip, int port) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connect failed"); return -1;
    }
    return sock;
}


//sending segment to peers
void send_segment(char *filename, char *ip, int port) {
    int sock = connect_to(ip, port);
    if (sock < 0) return;

    FILE *fp = fopen(filename, "rb");
    if (!fp) { perror("Cannot open file"); close(sock); return; }

    unsigned char buffer[DEFAULT_BUFLEN];
    size_t n;
    while ((n = fread(buffer, 1, DEFAULT_BUFLEN, fp)) > 0) {
        send(sock, buffer, n, 0);
    }
    fclose(fp);
    close(sock);
}

//getting segment from other peers
void recv_segment(char *filename, char *ip, int port) {
    int sock = connect_to(ip, port);
    if (sock < 0) return;

    FILE *fp = fopen(filename, "wb");
    unsigned char buffer[DEFAULT_BUFLEN];
    int n;
    while ((n = recv(sock, buffer, DEFAULT_BUFLEN, 0)) > 0) {
        fwrite(buffer, 1, n, fp);
        if (n < DEFAULT_BUFLEN) break; // kraj fajla
    }
    fclose(fp);
    close(sock);
}

int main() {

    pthread_t tid;
    int my_port = 40000; // port peer-a za primanje fajlove
    pthread_create(&tid, NULL, peer_server, &my_port);


    //1. Split original file
    split_file("files/test.txt", 512);


    //2. Register segment at tracker
    int sock = connect_to(TRACKER_IP, TRACKER_PORT);
    if(sock <  0) return 1;
    

    //Assume 3 segments
    for(int i = 0; i < 3; i++) {
        char msg[64];
        sprintf(msg, "UPLOAD %d", i);
        send(sock, msg, strlen(msg), 0);
    }
    close(sock);

    //3. Downloading segments (example)
    sock = connect_to(TRACKER_IP, TRACKER_PORT);
    if (sock < 0) return 1;
    for(int i = 0; i < 3; i++) {
        char msg[64];
        sprintf(msg, "GET %d", i);
        send(sock, msg, strlen(msg), 0);

        char resp[64];
        int n = recv(sock, resp, sizeof(resp), 0);
        resp[n] = '\0';
        printf("Segment %d owner: %s\n", i, resp);

        //seperate IP and port
        char ip[16];
        int port;
        sscanf(resp, "%15[^:]:%d", ip, &port);

        char filename[64];
        sprintf(filename, "segments/segment_%d_downloaded.dat", i);
        recv_segment(filename, ip, port);
    }
    close(sock);

    return 0;
}
