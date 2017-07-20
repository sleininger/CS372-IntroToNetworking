/**********************************************************************************
Author: Siara Leininger
CS 372
Program 2 - FTP File Transfer
Description: Implement 2-connection client-server network application
***I used code from my chatServer to begin this part of the project
Referenced[1]: https://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#faq
Referenced[2]: http://www.linuxhowtos.org/C_C++/socket.htm
Referenced[3]: http://softwareengineering.stackexchange.com/questions/297607/if-i-want-to-build-an-application-to-transfer-files-between-client-and-server-d
Referenced[4]: http://stackoverflow.com/questions/11254447/file-transfer-server-client-using-socket
Referenced[5]: http://codereview.stackexchange.com/questions/38071/client-server-application-for-file-transfer
Referenced[6]: http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
Referenced[7]: http://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
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
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define BUFF_LEN 256
#define BUFF_SIZE_LEN 256

/************************************************************************
 * struct addrinfo()
 * 
 * Creates pointer to address information
 * Arguments: address and port number
 * Function used from Project 1
 ************************************************************************/
struct addrinfo * createAddrInfo(char* ipAddress, char * port) {
    //Structures and error indicator
    int errorCheck;
    struct addrinfo hints, *res;

    //Clear old information, set TCP IPv4
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    //Check for errors
    if((errorCheck = getaddrinfo(ipAddress, port, &hints, &res)) != 0) {
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
 * Base of function used from Project 1
 ************************************************************************/
int createSocket(struct addrinfo * res) {
    int socketCreate;
    int errorCheck;
    int one = 1;
    
    //Return unless file descriptor is -1
    if((socketCreate = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        printf("Error creating socket.\n");
        exit(1);
    }

    //Connect socket to specified address
    /*if((errorCheck = connect(socketCreate, res->ai_addr, res->ai_addrlen)) == -1) {
        printf("Error connecting socket.\n");
        exit(1);
    }*/

    if(setsockopt(socketCreate, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0) {
    	printf("Setsockopt error.\n");
    	exit(1);
    }

    //Bind socket to port
    if(bind(socketCreate, res->ai_addr, res->ai_addrlen) == -1) {
    	close(socketCreate);
    	printf("Error binding socket.\n");
    	exit(1);
    }

    //Listen on port
    if(listen(socketCreate, 10) == -1) {
    	close(socketCreate);
    	printf("Error listening on socket.\n");
    	exit(1);
    }

    return socketCreate;
    
}

/************************************************************************
 * int createDataSocket()
 * 
 * Creates data socket for transfer
 * Arguments: ipAddress, data port
 ************************************************************************/
int createDataSocket(char* ipAddress, char * dataPort) {
	struct addrinfo * res = createAddrInfo(ipAddress, dataPort);

	//Make socket
	int dataSock;
	int errorCheck;

	//Create the data socket
	if((dataSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        printf("Error creating data socket.\n");
        exit(1);
    }

    //Connect the data socket
    if((errorCheck = connect(dataSock, res->ai_addr, res->ai_addrlen)) == -1) {
        printf("Error connecting data socket.\n");
        exit(1);
    }

    return dataSock;
}

/************************************************************************
 * void handleRequest()
 * 
 * Handles the request from the client
 * Arguments: socket from request
 ************************************************************************/
 void handleRequest(char* ip, int newSocket) {
 	int dataSocket, temp;
	char buffer[BUFF_LEN];
 	char port[BUFF_LEN];
 	char command[BUFF_LEN];
 	char ipAddr[BUFF_LEN];
 	char fileName[BUFF_LEN];
	int count;
	int clientPort;
	int isValidCommand = 1;
	int i, j;
	
 	//char errorFile[] = "Error: File does not exist.";
 	char * invalidMsg = "Invalid command.";
	char * validMsg = "OK";
	char * newLine = "\n";
	char * fileNotFound = "File not found";
	char * msg = NULL;
	
	strcpy(ipAddr, ip);//get ip address
	
	printf("ipAddr is %s.\n", ipAddr);

 	//Get port number for client
 	memset(buffer, 0, sizeof(buffer));
 	count = recv(newSocket, buffer, sizeof(buffer) - 1, 0);
	if (count < 0){
		perror("recv failed");
		return;
	}
	buffer[count] = '\0';
	printf("Buffer is %s.\n", buffer);
	
	//extract parameter
	count = sscanf(buffer,"%[^/]/%[^/]/%s", command, fileName, port);
	
	printf("Count = %d\n", count);
	
	if (count == 3){ //example -g/file name/1235
		count = sscanf(port, "%d", &clientPort);
		if (count != 1){
			msg = invalidMsg;
		}else{
			
			printf("Command = %s\n", command);		
			
			if(strcmp(command, "-g") == 0) {
				
				int fp = NULL;
				
				//Open the file
				fp = open(fileName, O_RDONLY);				
				
				if (fp == -1){
					msg = fileNotFound;
				}else{
					//buffer for file contents
					char buffer[BUFF_SIZE_LEN];

					printf("File requested.\n");
					
					
					send(newSocket, validMsg, strlen(validMsg), 0);
					
					//wait some seconds
					sleep(1); // sleep some seconds.

					//Start a data socket
					dataSocket = createDataSocket(ipAddr, port);

					printf("Sending file.\n");				
					
					while(1) {
						
						//Clear buffer
						memset(buffer, 0, sizeof(buffer));
					
						//Read data into buffer
						int bytesRead = read(fp, buffer, sizeof(buffer));
						if(bytesRead == 0) {
							break; //Done reading the file
						}

						if(bytesRead < 0) {
							printf("Error reading file.\n");
							
							break;
						}		

						//printf("DEBUG - bytesRead = %d.\n", bytesRead);
						
						buffer[bytesRead] = '\0';
						
						//P keeps track of place in buffer, decrement bytesRead to keep track of bytes left
						char *p = buffer;
						while(bytesRead > 0) {
							
							//printf("DEBUG - buffer = %s\n", buffer);
							//printf("DEBUG - bytesRead = %d.\n", bytesRead);
							
							int bytesWritten = send(dataSocket, p, bytesRead, 0);
							if(bytesWritten <= 0) {
								printf("Error writing file.\n");
								exit(1);
							}
							bytesRead -= bytesWritten;
							p += bytesWritten;
						}
					}
					
					printf("File sent.\n");
					
					//close file
					close(fp);				
					
					//close data socket
					close(dataSocket);				
				}
			}else{
				msg = invalidMsg;
			}
		}
	}else if (count == 2){ //example -l/1235
		
		strcpy(port, fileName);//the second parameter is port
		
		count = sscanf(port, "%d", &clientPort);
		
		if (count != 1){
			msg = invalidMsg;
		}else{
			
			printf("Command = %s\n", command);			
			
			//Check if request is -l, if so, handle
			if(strcmp(command, "-l") == 0) {
				printf("List of files requested.\n");
				printf("Sending directory list to port.\n");
				
				printf("Port = %s.\n", port);
				
				send(newSocket, validMsg, strlen(validMsg), 0);
				
				//wait some seconds
				sleep(1); // sleep some seconds.
				
				//Start a data socket
				dataSocket = createDataSocket(ipAddr, port);
				
				printf("Created DataSocket.\n");

				//Reference[6], how to list files in a directory
				DIR *d;
				struct dirent *dir;
				char * listInfo = NULL;

				d = opendir(".");
				if(d) {
					
					while((dir = readdir(d)) != NULL) {
						
						//if(dir->d_type == DT_REG) {
							listInfo = dir->d_name;
							
							if (strcmp(listInfo, ".") != 0 && strcmp(listInfo, "..") != 0){

								printf("Send listInfo = %s.\n", listInfo);

								if ((temp = send(dataSocket, listInfo, strlen(listInfo), 0)) < 0) {
									printf("Send directory error.\n");
								}

								//send new line
								if ((temp = send(dataSocket, newLine, strlen(newLine), 0)) < 0) {
									printf("Send directory error.\n");
								}		
							}							
							
						//}
					}
					
					
					printf("Directory sent.\n");
					close(dataSocket);
				}
			}
		}
	}else{
		msg = invalidMsg;
	}
	
	//If the command was not recognized
	if (msg != NULL){
		send(newSocket, msg, strlen(msg), 0);
 		printf("Invalid command.\n");
	}
 }

/**************************************************************************
                                MAIN
***************************************************************************/

int main(int argc, char ** argv) {
	
	int socketCreate, newSocket;
	struct sockaddr_storage client;
	socklen_t addrSize;
	
	char hoststr[BUFF_LEN];
	char portstr[BUFF_LEN];
	int rc;
	
	//Check that user entered correct arguments on start up
	if(argc != 2) {
		error("Invalid number of arguments.\n");
		exit(1);
	}

	//Create struct for address information
	struct addrinfo * res = createAddrInfo(NULL, argv[1]);
	//Start connection and create socket
	socketCreate = createSocket(res);
	//Check to see if it worked
	if(socketCreate == 1) {
		error("Socket creation failed.\n");
		exit(1);
	}

	printf("Server open, Port %s\n", argv[1]);

    //Connect to client and server socets
    //clientSocket = accept(socketCreate, (struct sockaddr*)NULL, NULL);

	//Wait for client connection
	//Get client size
	addrSize = sizeof(client);
    memset(&client, 0, sizeof(client));

	while(1) {
		
		printf("Waiting for client...\n");
		
		//Accept a new client
		newSocket = accept(socketCreate, (struct sockaddr*)&client, &addrSize);
        //newSocket = accept(socketCreate, NULL, NULL);
		//Continue to wait if nothing happens
		if(newSocket < 0) {
			printf("Connection failed.\n");
            break;
		}
		
		printf("Got connection\n");
		
		char hoststr[NI_MAXHOST];
		char portstr[NI_MAXSERV];

		rc = getnameinfo((struct sockaddr *)&client, 
			addrSize, hoststr, sizeof(hoststr), portstr, sizeof(portstr), 
			NI_NUMERICHOST | NI_NUMERICSERV);

				
		//If connection is there, handle the request.
		handleRequest(hoststr, newSocket);
		close(newSocket);
	}

	freeaddrinfo(res);

}
