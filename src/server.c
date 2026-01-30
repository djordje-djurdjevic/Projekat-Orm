#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include"segment.h"

#define BUFFER_LEN 512
#define N 512 //the length of segment in bytes
#define PORT 12345
#define MAX_THREADS 10
#define MAX_OWNERS 10

typedef struct {
    char ip[16];
    int port;
    int segmentIndex[14];
} Peer;

Peer segment_owners[MAX_OWNERS] = {0};
int segment_count[14] = {0};
 
void *ServerFunction(void *arg) {
    //prima string // GET X gde je X redni broj segmenta
    //proveri jel ima neki peer taj zadati segment ako ima 
    //server posalje tom peeru ip:port peer-a 
    //koji je zatrazio i segment x
    // taj peer posalje segment peer koji je zatrazio 
    //ako je Q break;
    int clientSocketFd = *(int *)arg; //lokalni socket
    int readSize;
    char buffer[BUFFER_LEN];

    while(1) {
        readSize = recv(clientSocketFd, buffer, BUFFER_LEN, 0);
        buffer[readSize] = '\0';

        if(strncmp(buffer, "GET", 3) == 0) {
            int seg;
            sscanf(buffer + 4, "%d", &seg); //get the segment needed
           // if() //ako nadje zadati semg 
            {

            }
           // else{
             //   char msg[] = "NO_PEER";

            //}
        }
    }

    return NULL;
}


//fali struktura da server zna koji peer poseduje koji segment


pthread_t threads[MAX_THREADS];

int main() {

    //spliting the file into the segments
    split_file("files/test.txt", N);

    int serverSocketFd;
    int clientSocketFd;

    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;

    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[BUFFER_LEN];

    //for threads
    int counter = 0;
    int clientT[MAX_THREADS];


    serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocketFd == 1) {
        perror("Socket creation failed.");
        return EXIT_FAILURE;
    }

    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port        = htons(PORT);

    if(bind(serverSocketFd, (struct sockaddr *)&serverAddress,
    sizeof(serverAddress)) < 0) {
        perror("Bind failed.");
        return EXIT_FAILURE;
    }
    else {
        printf("Socket bound successfully to [%s:%hu]\n\n",
        inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
    }

    //listen - max to wait before accept 5
    listen(serverSocketFd, 5);

    while (1) {
        clientT[counter] = accept(serverSocketFd,
        (struct sockaddr *)&clientAddress, (socklen_t *)&len);
        if (clientT[counter] < 0) {
            perror("Accept client failed.");
            continue; //ne pravi thread?
        }
        else {
            printf("Client [%s:%hu] connection accepted\n\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        }

        pthread_create(&threads[counter], NULL, ServerFunction, (void *)&clientT[counter]);
        pthread_detach(threads[counter]);
        counter++;

        if(counter > 9) {break;} //too much clients connected
    }

    close(serverSocketFd);
    printf("Server socket closed\n\n");

    return 0;
}