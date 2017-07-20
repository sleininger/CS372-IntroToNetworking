#Author: Siara Leininger
#CS 372
#Program 2 - FTP File Transfer
#Description: Implement 2-connection client-server network application
#Referenced: http://softwareengineering.stackexchange.com/questions/297607/if-i-want-to-build-an-application-to-transfer-files-between-client-and-server-d
#Referenced: http://www.bogotobogo.com/python/python_network_programming_server_client_file_transfer.php
#Referenced: https://docs.python.org/3/howto/sockets.html
#Referenced: https://pythonprogramming.net/ftp-transfers-python-ftplib/
#Referenced: http://stackoverflow.com/questions/25273107/python-client-server-transfer-txt-not-writing-to-file

import socket
import sys
import select

BUFF_LEN = 256

#################################################################
# initiateContact()
#
# Makes initial contact with the server
#################################################################
def initiateContact(host, port): 
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	try:
		s.connect((host, port))
	except:
		print 'Could not connect server'
		sys.exit()
	return s

#################################################################
# sendMessage()
#
# Send message to the server
#################################################################	
def sendMessage(s, msg):
	print "Message = ", msg
	s.sendall(msg)

#################################################################
# receiveDirectory()
#
# Gets information to list contents of directory
#################################################################
def receiveDirectory(s, recvPort):


	result = s.recv(BUFF_LEN)
	
	print "Result = ", result
	
	if result == "OK":			
		
		#open socket at data port to receive data
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.bind(('', int(recvPort)))
		s.listen(1)
		conn, addr = s.accept()
		while True:
			data = conn.recv(BUFF_LEN)			
			print data
			if not data:
				break
				
		s.close()
	else:	
		print result

#################################################################
# receiveFile()
#
# Receives file from server
# Referenced: http://stackoverflow.com/questions/25273107/python-client-server-transfer-txt-not-writing-to-file
#################################################################
def receiveFile(s, fileName, recvPort):

	result = s.recv(BUFF_LEN)
	
	print "Result = ", result
	
	if result == "OK":		
		
		#open socket at data port to receive data
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.bind(('', int(recvPort)))
		s.listen(1)
		conn, addr = s.accept()

		with open(fileName, 'w+') as outFile:
			while True:
				data = conn.recv(BUFF_LEN)
				if not data:
					break
				#print data
				#size = sys.getsizeof(data)
				#print "DEBUG - size = ", size
				outFile.write(data)

		s.close()		
	else:	
		print result
	

###############################################################
#                       MAIN
###############################################################
def main():

	#Check for correct number of arguments from user
	if(len(sys.argv) < 5):
		print 'Must include: hostname, port, action, receive port'
		sys.exit()

	#Assign values to host and port
	host = sys.argv[1]
	port = int(sys.argv[2])
	fileName = "" #Start blank in case just listing directory

	#Check to see if the given command is valid
	if((sys.argv[3] != '-l') and (sys.argv[3] != '-g')):
		#Print error message if invalid command
		print 'You must enter -l to list or -g to get.'
		sys.exit()
	#But if valid, then check which command user entered
	else:
		#If they want a list of files, there will only be 4 arguments
		#Fourth will be the receive port, assign value
		if(sys.argv[3] == '-l'):
			recvPort = sys.argv[4]
		#Only command option left is -g for get, assign appropriate values for 5 args
		else:
			fileName = sys.argv[4]
			recvPort = sys.argv[5]
			
	print 'Initiate contact...'

	#Ones all variables have been assigned, initiate contact with server
	server = initiateContact(host, port)

	#Let user know what's up.
	print 'Connected to remote host.'
	print 'Sending your request.'

	#Send appropriate request to server
	if(fileName == ""):
		msg = sys.argv[3] + "/" + recvPort
	else:
		msg = sys.argv[3] + "/" + fileName + "/" + recvPort

	#Call function to send message to server
	sendMessage(server, msg)

	#Then call functions to receive info, depending on command
	if(sys.argv[3] == '-l'):
		print 'receiveDirectory.'
		receiveDirectory(server, recvPort)
	else:
		print 'receiveFile.'
		receiveFile(server, fileName, recvPort)
main()
