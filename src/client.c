#include <stdio.h>          //printf
#include <stdlib.h>         //exit codes
#include <string.h>         //strlen
#include <sys/socket.h>     //socket
#include <arpa/inet.h>      //inet_addr
#include <fcntl.h>          //for open
#include <unistd.h>         //for close

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_LEN 512 


//treba thread
void posalji_segment() {
    //recv od servera
    //send segment to peer 
}

//


int main() {

    int clientSocketFd;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_LEN];

    //create Socket
    clientSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocketFd == -1) {
        perror("Socket creation failed.");
        return EXIT_FAILURE;
    }
    //else ?

    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);

    if(connect(clientSocketFd, (struct sockaddr *)&serverAddress,
    sizeof(serverAddress)) < 0) {
        perror("Failed to connect");
        return EXIT_FAILURE;
    }
    else
    {
        printf("Successfully connected to server [%s:%hu]\n\n",
        inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
    }
    

    /*
    int element_choice;
    printf("Q to end session. Anything else to continue: \n");
    while(1) {
    
        printf("Choose your element: \n");
        printf("0 => WATER     1 => FIRE      2 => AIR      3 => EARTH: \n");

        do{
            scanf("%d", &element_choice);
            if(element_choice < 0 || element_choice > 3) {printf("Choose a value whitin the range.\n");}

        }while(element_choice < 0 || element_choice > 3);
        
        //element[strcspn(element, "\n\r")] = "\0";
    
        if(send(clientSocketFd , &element_choice , sizeof(element_choice), 0) < 0)
        {
            printf("Message sending failed");
        }
        else
        {
            printf("Element \"%d\" successfully sent to server\n\n", element_choice);
        }
    }
    */
    

    close(clientSocketFd);
    printf("Socket closed\n\n");
    return 0;
}

