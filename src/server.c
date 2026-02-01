#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include"segment.h"
#include <stdbool.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 16666
#define PEER_SERVER_PORT 20000
#define BUFFER_LEN 512
#define N 512 //the length of segment in bytes
#define MAX_THREADS 10
#define MAX_OWNERS 10
#define SEGMENTS 6

typedef struct {
    char ip[16];    // "xxx.xxx.xxx.xxx\0"
    int port;
    bool segmentIndex[SEGMENTS];
} Peer;

int segmentOwnerCounter = 0;
Peer segment_owners[MAX_OWNERS] = {0};
int segment_count[SEGMENTS] = {0};


typedef struct {
    int socket;
    int peerId;
}ThreadArgs;

ThreadArgs threadArgs[MAX_OWNERS];

pthread_t threads[MAX_THREADS];
pthread_mutex_t mutex;
 
void *ServerFunction(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg; 
    int clientSocketFd = args->socket;      //local socket
    int peerId = args->peerId;
    int readSize;
    char buffer[BUFFER_LEN];

    while(1) {

        readSize = recv(clientSocketFd, buffer, BUFFER_LEN, 0);
        if(readSize < 0) {
            perror("An error occured.\n");
            break;
        }
        else if(readSize == 0) {
            printf("Client disconnected.\n");
            break;
        }
        buffer[readSize] = '\0';
        
        if(strncmp(buffer, "GET", 3) == 0) {
            char ipSERVER_PORT[32];
            int seg;
            sscanf(buffer + 4, "%d", &seg); //get the segment needed


            bool found = false;
            //race coniditon segment_count and segment_owners[i].segmentIndex[seg]
            pthread_mutex_lock(&mutex);
            if(segment_count[seg]  > 0) //
            {
                for(int i = 0; i < MAX_OWNERS; i++) {

                    if(segment_owners[i].segmentIndex[seg]) { 
      
                        snprintf(ipSERVER_PORT, sizeof(ipSERVER_PORT), "%s:%d", 
                        segment_owners[i].ip, segment_owners[i].port);
                        found = true;
                        break;                     
                    }
                }
            }
            pthread_mutex_unlock(&mutex);

            if(found) {
                if(send(clientSocketFd, ipSERVER_PORT, strlen(ipSERVER_PORT), 0) < 0) {
                    printf("Sending message failed.\n");
                }       
                else {
                    printf("Message %s successfully sent{PEER}.\n", ipSERVER_PORT);
                }   
            }
            else{ //none of the peers have the segment server sends ip:prot
                //sending server ip:SERVER_PORT
                snprintf(ipSERVER_PORT, sizeof(ipSERVER_PORT), "%s:%d", SERVER_IP, PEER_SERVER_PORT);
                if(send(clientSocketFd, ipSERVER_PORT, strlen(ipSERVER_PORT), 0) < 0) {
                    printf("Sending message failed.\n");
                }       
                else {
                    printf("Message %s successfully sent{SERVER}.\n", ipSERVER_PORT);
                }     
            }
        }
        else if(strncmp(buffer, "UPLOAD", 6) == 0) {
            int i;
            sscanf(buffer + 7, "%d", &i);

            pthread_mutex_lock(&mutex);
            segment_owners[peerId].segmentIndex[i] = true; //the peer has the i-th segment
            ++segment_count[i];
            pthread_mutex_unlock(&mutex);
        }
    }
    close(clientSocketFd);
    return NULL;
}

/*printf("[DEBUG] Peer %s:%d registered with ID %d\n", 
       segment_owners[peerId].ip, segment_owners[peerId].port, peerId);*/

int main() {

    pthread_mutex_init(&mutex, NULL); //ovo sutra dodati funkciju
    //pthread_create(&peerThread, NULL, PeerServer, &serverPeerPort);
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
    serverAddress.sin_port        = htons(SERVER_PORT);

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
            continue;
        }
        else {
            printf("Client [%s:%hu] connection accepted\n\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        }


        //Save ip:port of peers for later use
        strcpy(segment_owners[segmentOwnerCounter].ip , inet_ntoa(clientAddress.sin_addr));
        segment_owners[segmentOwnerCounter].port = ntohs(clientAddress.sin_port);

        threadArgs[segmentOwnerCounter].socket = clientT[counter];
        threadArgs[segmentOwnerCounter].peerId = segmentOwnerCounter;

        ++segmentOwnerCounter; //thread safe


        
        pthread_create(&threads[counter], NULL, ServerFunction, (void *)&threadArgs[segmentOwnerCounter - 1]);
        pthread_detach(threads[counter]);
        counter++;

        if(counter > 9) {break;} //too much clients connected
    }


    pthread_mutex_destroy(&mutex);
    close(serverSocketFd);
    printf("Server socket closed\n\n");

    return 0;
}