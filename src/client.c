#include <stdio.h>          //printf
#include <stdlib.h>         //exit codes
#include <string.h>         //strlen
#include <sys/socket.h>     //socket
#include <arpa/inet.h>      //inet_addr
#include <fcntl.h>          //for open
#include <pthread.h>
#include <unistd.h>         //for close

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 16666
#define BUFFER_LEN 512 
#define MAX_THREADS 10
#define SEGMENTS 6

void *PeerServer(void *arg) {

    int port = *(int *)arg;
    int sockFd = socket(AF_INET, SOCK_STREAM, 0 );
    if(sockFd == -1) {perror("Socket creation failed."); return NULL;}
    struct sockaddr_in address;    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if(bind(sockFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind error.");
        return NULL;
    }
    listen(sockFd, 5);

    while(1) {
        int clientFd = accept(sockFd, NULL, NULL);
        if (clientFd < 0) {
            perror("accept failed");
            continue;
        }
        //receive numb of segm, onda parsiranje
        /*int seg;
        recv(clientFd, &seg, sizeof(int), 0);

        char filename[64];
        sprintf(filename, "segments/segment_%d.dat", seg);
        FILE *fp = fopen(filename, "rb");
        */
        unsigned char buffer[BUFFER_LEN];
        FILE *fp = fopen("segments/segments_0.dat", "rb");
        size_t n;
        while((n = fread(buffer,  1, sizeof(buffer), fp)) > 0) {
            send(clientFd, buffer, n, 0);
        }
        fclose(fp);
        close(clientFd);
    }

    close(sockFd);
    return NULL;
}

int connect_to(char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        printf("Scoket creation failed.");
        return -1;
    }
    struct sockaddr_in address;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if(connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Connect failed."); 
        return -1;
    }
    else{
        printf("Successfully connected to server [%s:%hu]\n\n",
        inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    }
    return sock;
}

void recv_segment(char *filename, char *ip, int port) {
    int sock = connect_to(ip, port);
    if (sock <  0) return;

    FILE *fp = fopen(filename, "wb"); //write binary
    unsigned char buffer[BUFFER_LEN];
    int n;
    while((n = recv(sock, buffer, BUFFER_LEN, 0)) > 0) {
        fwrite(buffer, 1, n, fp);
    }
    fclose(fp);
    close(sock);
}


pthread_t thread;

int main() {
    int peerPort = 20000 + rand() % 1000; //fiksno?
    pthread_create(&thread, NULL, PeerServer, (void *)&peerPort);
    pthread_detach(thread);

    int clientSocketFd = connect_to(SERVER_IP, SERVER_PORT);
    if(clientSocketFd <  0) {return EXIT_FAILURE;}


    char buffer[BUFFER_LEN];
    char message[32]; 
    int segment;
    printf("-1 to disconnect.\n");
    while(1) {
        printf("Choose the segment: \n");

        do{
            scanf("%d", &segment);
        }while(segment < -1 || segment > SEGMENTS); 
        if(segment == -1) {break;}
        snprintf(message, sizeof(message), "GET %d", segment);

        //sending which segment the port wants
        if(send(clientSocketFd, message, strlen(message), 0) < 0) {
            printf("Sending message failed.\n");
        }
        else {
            printf("Message %s successfully sent.\n", message);
        }

        //recive ip:port
        int n = recv(clientSocketFd, buffer, sizeof(buffer) - 1, 0);
        if(n < 0) {
            perror("Recv error");
            return EXIT_FAILURE;
        }
        char ip[16];
        int port;
        if (sscanf(buffer, "%15[^:]:%d", ip, &port) != 2) {
            printf("Neispravan format peer adrese\n");
        }   //oke?


        char filename[64];
        sprintf(filename, "segments/segment_%d_downloaded.dat", segment);
        recv_segment(filename, ip, port);
        //ovde POSLE MORA UPLOAD X
    }    

    close(clientSocketFd);
    printf("Socket closed\n\n");
    return 0;
}

