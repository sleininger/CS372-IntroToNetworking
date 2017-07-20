/**********************************************************************************
Author: Siara Leininger
CS 372
Program 1 - Chat Client
Description: Implement a client-server network chat application
Referenced: https://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#faq
Referenced: http://www.linuxhowtos.org/C_C++/socket.htm
***********************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>

//Gets user's "handle" by initial query
//DIsplays handle as a prompt on Host B, and will prepend it to all messages to host A 
//Sends an initial message to chatserve on host A : PORTNUM
//the command /quit will close connection on either side
//If not closed, repeat process


/************************************************************************
 * void getHandle()
 * 
 * Gets the user's handle
 * Arguments: character pointer to handle
 ************************************************************************/
void getHandle(char * handle){
	printf("Please enter a 10 character or less username: ");
	scanf("%s", handle);
    strcat(handle, ": ");
}

/************************************************************************
 * struct addrinfo()
 * 
 * Creates pointer to address information
 * Arguments: address and port number
 ************************************************************************/
struct addrinfo * createAddrInfo(char * host, char * port) {
    //Structures and error indicator
    int errorCheck;
    struct addrinfo hints, *res;

    //Clear old information, set TCP IPv4
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    //Check for errors
    if((errorCheck = getaddrinfo(host, port, &hints, &res)) != 0) {
        printf("Incorrect IP or Port.\n");
        exit(1);
    }

    return res;
}

/************************************************************************
 * int createSocket()
 * 
 * Creates socket and sets up server configuration
 * Arguments: pointer to address information
 ************************************************************************/
int createSocket(struct addrinfo * res) {
    int socketCreate;
    int errorCheck;
    
    //Return unless file descriptor is -1
    if((socketCreate = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        printf("Error creating socket.\n");
        return 1;
    }

    //Connect socket to specified address
    if((errorCheck = connect(socketCreate, res->ai_addr, res->ai_addrlen)) == -1) {
        printf("Error connecting socket.\n");
        return 1;
    }

    return socketCreate;
    
}

/************************************************************************
 * void sendMessage
 * 
 * Reads input, sends over connection
 * Arguments: client's handle, socket to send message
 ************************************************************************/
int sendMessage(int socketCreate, char handle[12]) {
    char fullMessage[512];
    char content[502];
    int errorCheck;

    //Make sure nothing left over from previous time
    memset((char *)fullMessage, '\0', sizeof(fullMessage));
    memset((char *)content, '\0', sizeof(content));
 
    //Get input
    read(0, content, 512);
    //End of message
    strcat(content, "\0");
    //Gets message ready to send
    strcat(fullMessage, handle);
    strcat(fullMessage, content);
    
    //Check to see if /quit entered
    if(strncmp(content, "/quit\n", 5) != 0) {
        //Send to server
        errorCheck = write(socketCreate, fullMessage, strlen(fullMessage)-1);
        if(errorCheck < 0) {
            error("Send error.\n");
        }
        return 0;
    }
    //Else server ended the chat
    else {
        //Chat ended
        printf("The chat has been ended.\n\n");
        write(socketCreate, "/quit", 5);
        return -2;
    }

    return 0;

}

/************************************************************************
 * void receiveMessage()
 * 
 * Receives message from server
 * Arguments: int for the socket
 ************************************************************************/
 int receiveMessage(int socketCreate) {
    char messageRec[512];
    int errorCheck;

    //Make sure cleared from previous time
    memset((char *)messageRec, '\0', sizeof(messageRec));

    //Check for errors
    errorCheck = read(socketCreate, messageRec, 512);
    if (errorCheck < 0) {
        error("Receive error.");
        return -1;
    }
    //Check for /quit message and if received, send one back
    else if(strncmp(messageRec, "/quit\n", 5) == 0) {
        printf("The chat has ended.\n");
        write(socketCreate, "/quit", 5);
        return -2;
    }
    //Print the message
    else {
        printf("%s\n", messageRec);
    }

    return 0;
 }


/**************************************************************************
                                MAIN
***************************************************************************/

int main(int argc, char * argv[]) {
    int serverPort, socketCreate, testConnect;
    char * serverHost;
    char userHandle[12];

    //Check that user entered correct arguments on start up
    if(argc < 2) {
        error("Invalid port and host number.\n");
        exit(1);
    }
    
    //Get Port and Host
    serverPort = atoi(argv[2]);
    serverHost = argv[1];



    //Create struct for address information
    struct addrinfo * res = createAddrInfo(argv[1], argv[2]);
    //Start connection and create socket
    socketCreate = createSocket(res);
    //Check to see if it worked
    if(socketCreate == 1) {
        error("Socket creation failed.\n");
        exit(1);
    }

    //Get user's handle
    getHandle(userHandle);

    //Start the chat
    testConnect = 0;
    while(testConnect != -2) {
        testConnect = sendMessage(socketCreate, userHandle);
        if(testConnect == -1) {
            exit(1);
        }
        else if(testConnect == -2) {
            printf("The chat session has ended.\n");
            close(socketCreate);
            exit(0);
        }

        testConnect = receiveMessage(socketCreate);
        if(testConnect == -1) {
            exit(1);
        }
        else if(testConnect == -2) {
            printf("The chat session has ended.\n");
            close(socketCreate);
            exit(0);
        }
    }

    freeaddrinfo(res);

}
