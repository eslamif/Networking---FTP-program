#Frank Eslami, CS 372, Project 2
#ftclient

#A simple file transfer system between a server and client. This is the client application.

#The following Python socket networking guide was used as reference:
#http://www.binarytides.com/code-chat-application-server-client-sockets-python/

import socket, select, string, sys, os


# ========= FUNCTION DEFINITIONS ========
# =======================================

#Print message to user
#message: string
#newLine: 1 = add new line; 0 = don't add new line
def prompt(message, newLine):
	sys.stdout.write("ftclient> " + message);	
	if (newLine == 1):
		sys.stdout.write("\n");	
	sys.stdout.flush();


# =========== MAIN FUNCTION =============
# =======================================
if __name__ == "__main__":
	#Validate arguments	
#	print len (sys.argv)
	if (len (sys.argv) == 5):
		#Usage: python ftclient.py <server> <port_control> <command> <port_data>
		server = sys.argv[1]
		port_control = int (sys.argv[2])
		command = sys.argv[3]
		port_data = int (sys.argv[4])
#		print server, port_control, command, port_data

		#Create data connection request for server
		message = command + " " + str(port_data)
#		print message
	elif (len (sys.argv) == 6):
		#Usage: python ftclient.py <server> <port_control> <command> <file> <port_data>
		server = sys.argv[1]
		port_control = int (sys.argv[2])
		command = sys.argv[3]
		file_data = sys.argv[4]
		port_data = int (sys.argv[5])
#		print server, port_control, command, file_data, port_data

		#Create data connection request for server
		message = command + " " + file_data + " " + str(port_data)
#		print message
	else:
		print "Invalid arguments. Usage:"
		print "python ftclient.py <server> <port_control> <command> <port_data>"
		print "Or"
		print "python ftclient.py <server> <port_control> <command> <file> <port_data>"
		sys.exit()

	#Create control connection to server	
	sock_1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock_1.settimeout(2)
	     
	# connect to server 
	try :
	    sock_1.connect((server, port_control))
	except :
	    print 'Unable to connect server'
	    sys.exit()
	
	#Tell user connection successful 
	prompt("Control connection to server " + server + " on port_control " + str(port_control) + " established.", 1)
	prompt("Ready to send file transfer requests to ftserver\n", 1)

	#Send data connection command/file/port to server
	sock_1.send(message)	
	prompt("File info request sent to ftserver. Waiting for response...", 1)
#	print "Message to server =", message

	#Wait for server data connection handshake
	buffer_data = sock_1.recv(100)
	if (buffer_data != "success"):
		print ("Error: server unable to create data connection")
		sock_1.close()
		sys.exit()
	#print "message from server =", buffer_data

	#Create data connection to server	
	sock_2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock_2.settimeout(2)
	     
	# connect to server 
	try :
		sock_2.connect((server, port_data))
	except :
		print 'Unable to connect server'
		sock_1.close()
		sys.exit()
	prompt("Data connection to server successful", 1)

	#Get file info from server
	prompt("Waiting for file info...", 1)

	if (len (sys.argv) == 5):	 #directory listing request
		buffer = sock_2.recv(512)
#		print "message from server =", buffer
	
		#Display directories to user
		print
		prompt("Directory listing from server received:", 1)
		split = buffer.split()
		for s in split:
			print " ", s
	elif (len (sys.argv) == 6):		#get file from server
		#If file does not exist on server
		buffer = sock_1.recv(512)
		if (buffer == "file_dne"):
			prompt("File does not exist. Exiting app...", 1)
			sock_2.close()
			sock_1.close()
			sys.exit()
		else:
			#File exists. Send success response to server
#			print "File size is", buffer
			message = "success"
			sock_1.send(message)	
			
			#Extract file size and name from buffer
			split = buffer.split()
			file_size = int (split[0])
			file_name = split[1]
#			print file_size
#			print file_name

			#Obtain file content from server
			file_data = sock_2.recv(file_size)
#			print "File contents is", file_data	

			#See if file already exists
			if (os.path.isfile (file_name)):
				prompt("File " + file_name + " already exists. Enter new name: ", 0)
				file_name = raw_input();
				prompt("Saving file as " + file_name + "...", 1);
	
			#Save file content
			file_new = open (file_name, 'w')
			file_new.write (file_data)
			file_new.close()
			prompt ("File successfully saved as " + file_name, 1)
			
	sock_2.close()
	sock_1.close()





