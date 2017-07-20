#Author: Siara Leininger
#CS 372
#Program 1 - Chat Client
#Description: Implement a client-server network chat application
#Referenced: https://docs.python.org/2/howto/sockets.html, http://www.tutorialspoint.com/python/python_networking.htm

from socket import *
from sys import *

###############################################################
# getHandle()
#
#Gets the user's handle
###############################################################
def getHandle():
    print "Please enter a username 10 characters or less: "
    handle = raw_input()
    handle = handle + ": "

    return handle

###############################################################
# createSocket()
#
#Takes arguments from command line, starts socket
###############################################################
def createSocket():
    if len(argv) < 2:
        print "Port number needed.\n"
        exit(1)

    #Read port number
    port = int(argv[1])

    #Set up socket
    serverSocket = socket(AF_INET, SOCK_STREAM)
    serverSocket.bind(('',port))    
    serverSocket.listen(5)

    return serverSocket

###############################################################
# sendMessage()
#
#Takes in input from user and sends message to client
###############################################################
def sendMessage(connectionSocket, handle):
    #Get input from user
    content = raw_input()
    fullMessage = handle + content

    #Check to see if /quit
    if content == "/quit":
        connectionSocket.send("/quit")
        return 1

    #Otherwise, send message to client
    connectionSocket.send(fullMessage)
    return 0

###############################################################
# receiveMessage()
#
#Receives and displays message from client
###############################################################
def receiveMessage(connectionSocket):
    #Receive the message with appropriate size
    message = connectionSocket.recv(512)

    #Check for /quit
    if message == "/quit":
        print "The chat has been ended.\n"
        return 1

    #Otherwise, display message from client
    print message
    return 0

###############################################################
#                       MAIN
###############################################################
def main():
    serverSocket = createSocket()
    print "Chat opened.\n"

    #Get handle
    handle = getHandle()

    #Set up loop to listen for connection
    while 1: 
        #Accept incoming socket request 
        connectionSocket, address = serverSocket.accept()

        while 1:
            #Receive message first, and check for quit
            if receiveMessage(connectionSocket) == 1:
                break
            #Send message, check for quit
            if sendMessage(connectionSocket, handle) == 1:
                break

        #Close connection if someone quit
        connectionSocket.close()


main()
