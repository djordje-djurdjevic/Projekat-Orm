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
#define SERVER_PORT 18888
#define PEER_SERVER_PORT 22222
#define BUFFER_LEN 512
#define N 512 //the length of segment in bytes
#define MAX_THREADS 10
#define MAX_OWNERS 10
#define SEGMENTS 5

typedef struct {
    char ip[16];    // "xxx.xxx.xxx.xxx\0"
    int port;
    bool segmentIndex[SEGMENTS];
} Peer;

Peer segmentOwners[MAX_OWNERS] = {0};
int segmentCount[SEGMENTS] = {0};


typedef struct {
    int socket;
    int peerId;
}ThreadArgs;

ThreadArgs threadArgs[MAX_OWNERS];

pthread_t threads[MAX_THREADS];
pthread_t peerThread;
pthread_mutex_t mutex;

int CreateSocket();
int BindSocket(int, struct sockaddr_in  *);
 
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
            char IP_PORT[32];
            int seg;
            sscanf(buffer + 4, "%d", &seg); //get the segment needed


            bool found = false;
            //race coniditon segment_count and segment_owners[i].segmentIndex[seg]
            pthread_mutex_lock(&mutex);
            if(segmentCount[seg]  > 0) //
            {
                for(int i = 0; i < MAX_OWNERS; i++) {

                    if(segmentOwners[i].segmentIndex[seg]) { 
      
                        snprintf(IP_PORT, sizeof(IP_PORT), "%s:%d", 
                        segmentOwners[i].ip, segmentOwners[i].port);
                        found = true;
                        break;                     
                    }
                }
            }
            pthread_mutex_unlock(&mutex);

            if(found) {
                if(send(clientSocketFd, IP_PORT, strlen(IP_PORT), 0) < 0) {
                    printf("Sending message failed.\n");
                }       
                else {
                    printf("Message %s successfully sent{PEER}.\n", IP_PORT);
                }   
            }
            else{ //none of the peers have the segment server sends ip:prot
                //sending server ip:SERVER_PORT
                snprintf(IP_PORT, sizeof(IP_PORT), "%s:%d", SERVER_IP, PEER_SERVER_PORT);
                if(send(clientSocketFd, IP_PORT, strlen(IP_PORT), 0) < 0) {
                    printf("Sending message failed.\n");
                }       
                else {
                    printf("Message %s successfully sent{SERVER}.\n", IP_PORT);
                }     
            }
        }
        else if(strncmp(buffer, "UPLOAD", 6) == 0) {
            int i;
            sscanf(buffer + 7, "%d", &i);
            pthread_mutex_lock(&mutex);
            if (!segmentOwners[peerId].segmentIndex[i]) { //if it was already added
                segmentOwners[peerId].segmentIndex[i] = true;
                segmentCount[i]++;
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    close(clientSocketFd);

    for(int i = 0; i < SEGMENTS; i++) {
        segmentOwners[peerId].segmentIndex[i] = false; 
    }

    return NULL;
}

void *PeerServer(void *arg) {
    
    int port = *(int *)arg;
    int serverSocketFd;
    int clientFd;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    socklen_t len = sizeof(struct sockaddr_in);

    serverSocketFd = CreateSocket(); 

    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port        = htons(port);

    int opt = 1; //fix  <=>  Bind failed: Address already in use
    setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


    if(BindSocket(serverSocketFd, &serverAddress) < 0) {return NULL;} 
    listen(serverSocketFd, 5);//listen - max to wait before accept 5
    
    while(1) {
        clientFd = accept(serverSocketFd,
        (struct sockaddr *)&clientAddress, (socklen_t *)&len);
        if (clientFd < 0) {
            perror("Accept client failed.\n");
            continue;
        }
        else {
            printf("Client [%s:%hu] connection accepted\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        }


        int seg,net_seg;
        int temp = recv(clientFd, &net_seg, sizeof(int), 0);
        if(temp <  0) {continue;}
        seg = ntohl(net_seg); //doesnt need to be implemented for loopback address

        char filename[64];
        sprintf(filename, "segments/segment_%d.dat", seg);
        FILE *fp = fopen(filename, "rb");
        
        unsigned char buffer[BUFFER_LEN];
        size_t n;
        while((n = fread(buffer,  1, sizeof(buffer), fp)) > 0) {
            send(clientFd, buffer, n, 0);
        }
        fclose(fp);
        close(clientFd);
        
    }

    return NULL;
}




int main() {

    pthread_mutex_init(&mutex, NULL); 
    int tempPort = PEER_SERVER_PORT;
    pthread_create(&peerThread, NULL, PeerServer, (void *)&tempPort);
    
    //spliting the file into the segments
    split_file("files/test.txt", N);


    int serverSocketFd;

    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;

    socklen_t len = sizeof(struct sockaddr_in);

    int segmentOwnerCounter = 0;
    int peerPort;

    //for threads
    int counter = 0;
    int clientT[MAX_THREADS];


    serverSocketFd = CreateSocket(); 
    if (serverSocketFd < 0) {return EXIT_FAILURE;}

    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port        = htons(SERVER_PORT);

    int opt = 1; //fix  <=>  Bind failed: Address already in use
    setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(BindSocket(serverSocketFd, &serverAddress) < 0) {return EXIT_FAILURE;} 
    listen(serverSocketFd, 5);     //listen - max to wait before accept 5

    
    while (1) {

        clientT[counter] = accept(serverSocketFd,
        (struct sockaddr *)&clientAddress, (socklen_t *)&len);
        if (clientT[counter] < 0) {
            perror("Accept client failed.\n");
            continue;
        }
        else {
            printf("Client [%s:%hu] connection accepted\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        }

        int temp = recv(clientT[counter], &peerPort, sizeof(int), 0);
        if(temp <  0) {continue;}

        pthread_mutex_lock(&mutex);
        int myId = segmentOwnerCounter;
        segmentOwnerCounter++;
        pthread_mutex_unlock(&mutex);

        //Save ip:port of peers for later use
        inet_ntop(AF_INET, &clientAddress.sin_addr, //inet_top instead of inet_ntoa (thread safe), strcpy(segment_owners[myId].ip, inet_ntoa(clientAddress.sin_addr)); 
                            segmentOwners[myId].ip,
                            sizeof(segmentOwners[myId].ip));
        segmentOwners[myId].port = peerPort;

        threadArgs[myId].socket = clientT[counter];
        threadArgs[myId].peerId = myId;


        
        pthread_create(&threads[counter], NULL, ServerFunction, (void *)&threadArgs[segmentOwnerCounter - 1]);
        pthread_detach(threads[counter]);
        counter++;

        if (counter >= MAX_THREADS) break; //too many clients
    }


    pthread_mutex_destroy(&mutex);
    close(serverSocketFd);
    printf("Server socket closed\n");

    return 0;
}



int CreateSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Socket creation failed.\n");
        return EXIT_FAILURE;
    }

    return sock;
}

int BindSocket(int serverSocketFd, struct sockaddr_in  *serverAddress) {
    if(bind(serverSocketFd, (struct sockaddr *)serverAddress,
    sizeof(*serverAddress)) < 0) {
        perror("Bind failed.\n");
        return -1;
    }
    else {
        printf("Socket bound successfully to [%s:%hu]\n",
        inet_ntoa(serverAddress->sin_addr), ntohs(serverAddress->sin_port));
    }
    return 0;
}