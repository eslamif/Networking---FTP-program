all: server 

server: ftserver.cpp
		g++ -Wall -o ftserver ftserver.cpp
clean:
		rm -f ftserver 
