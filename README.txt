Frank Eslami
CS 372 Project 2
README.txt


Note to Grader
=================
Mr. Redfield allowed me to turn this in today for full credit. Please see the included email, titled "email.pdf."


How to compile the programs
============================
1. Place all 3 files in the same directory:
     1) ftserver.cpp: server
     2) ftclient.py: client
     3) makefile
     4) shortfile.txt

2. Type "make" without quotations to compile the programs.


How to launch the programs
===========================
3. Start ftserver like so:
     ftserver <port>
     Port represents a free port number that can be used.
     ftserver will now be listening for incoming connections.
     The command prompt will show the hostname and port of ftserver.

4. Start ftclient in one of two ways:
     a) python ftclient.py <server> <port_control> <command> <port_data>
     b) python ftclient.py <server> <port_control> <command> <file> <port_data>

     <server> is the hostname of ftserver (provided by ftserver command prompt).
     <port_control> is the port number of ftserver (provided by ftserver command prompt).
     <command> is either -l or -g (if -g is used, please use second call 'b')
     <file> is the name of the text file you wish to receive from the server
     <port_data> is the port number you must specify to open a data connection for the file transfer
     

How the programs communicate
=============================
Once steps 1-4 above are complete the server and client will begin communicating automatically.


References
===========
http://beej.us/guide/bgnet/
http://www.binarytides.com/code-chat-application-server-client-sockets-python/
http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c


Testing Machine
=================
The OSU flip server. Two windows were used, one for the client and the other for the server.
