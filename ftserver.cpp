//Frank Eslami, cs372, Project 2
//ftserver
//A simple file transfer system between a server and client. This is the server application.

/*
The following sites were used as references:
http://beej.us/guide/bgnet/
http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
*/

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <fstream>

using namespace std;

#define BACKLOG 1 // how many pending connections queue will hold

/* ********** FUNCTION DECLARATIONS ********** */
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);


int main(int argc, char *argv[]) {
	int sock_1, sock_2, sock_3; // listen on sock_fd, new connection on sock_2
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	char buffer[502] = {0};
	char tmp_buffer[502] = {0};
	char message[502] = {0};
	char * port_control, * port_data;

	//Ensure port number is provided by user
	if (argc != 2) {
 	    fprintf(stderr,"Invalid arguments. Usage: ftserver port\n");
 	    exit(1);
 	}
	port_control = argv[1];	//get port number

	//Prepare socket
	memset(&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port_control, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sock_1 = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
		perror("server: socket");
		continue;
		}

		if (setsockopt(sock_1, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sock_1, p->ai_addr, p->ai_addrlen) == -1) {
			close(sock_1);
			perror("server: bind");
			continue;
		}
		break;
	}
/*
	freeaddrinfo(servinfo); // all done with this structure
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
*/
	if (listen(sock_1, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	//Tell user server is listening on specified address	
	char hostname[1024];
	gethostname(hostname, 1024);

	printf("\nftserver> Listening on the following control address:\n");
	printf("ftserver> Hostname: %s\n", hostname);
	printf("ftserver> Port: %s\n", port_control);

	//Accept connection
	int read_size = 0;
	while(1) { 
		sin_size = sizeof their_addr;
		sock_2 = accept(sock_1, (struct sockaddr *)&their_addr, &sin_size);
		if (sock_2 == -1) {
			perror("accept");
			continue;
		}

		//Echo connection to client
		inet_ntop(their_addr.ss_family,	get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("\nftserver> Got control connection request from %s\n", s);

		//Spawn child process to handle connection
		if (!fork()) { // this is the child process
			close(sock_1); // child doesn't need the listener

			printf("ftserver> Control connection with ftclient began. Waiting for file transfer request...\n");
//>>>>> HERE
			//Wait for data connection info from client
			memset(buffer, 0, sizeof(buffer));
			while(1) {	
				if((read_size = recv(sock_2, buffer, 499, 0)) == 1) {
					perror("recv");
					exit(1);
				}
				else if (read_size == 0) {
					printf("\nftserver> Connection to client closed\n");
					printf("\nftserver> Listening on the following address:\n");
					printf("ftserver> Hostname: %s\n", hostname);
					printf("ftserver> Port: %s\n", port_control);
					break;
				}
				buffer[read_size] = '\0';
				printf("ftserver> File transfer request received: %s\n", buffer);

				//Extract command, file, and data port from client's message
				char * command = {0};
				char * file_name = {0};

				memcpy(tmp_buffer, buffer, strlen(buffer) + 1);
//				printf ("tmp_buffer = %s\n", tmp_buffer);
				char * tokens[4];
				int tokens_counter = 0;
				int tokens_size = 0;
				char * token = strtok (tmp_buffer, " ");
				tokens_size += strlen( tokens[tokens_counter]) * sizeof(tokens[0]);
				
				//Get tokens
				while (token != NULL) {
					tokens[tokens_counter] = (char *) malloc(strlen(token) + 1);
					strcpy(tokens[tokens_counter], token);
					tokens_size += strlen( tokens[tokens_counter]) * sizeof(tokens[0]);
//					printf(">>>>>> %s\n", tokens[tokens_counter]);
//					printf(">>>>>> %ld\n", strlen( tokens[tokens_counter]));
//					printf(">>>>>> %i\n", tokens_size);
					token = strtok (NULL, " ");
//					printf("%s\n", tokens[tokens_counter]);
					tokens_counter++;
				}

				//Set tokens to variables
//				printf("tokens_counter = %i\n", tokens_counter);
				if (tokens_counter == 3) {
					command = tokens[0];
					file_name = tokens[1];
					port_data = tokens[2];	

//					printf("command = %s\n", command);
//					printf("file_name = %s\n", file_name);
//					printf("port_data = %s\n", port_data);
				}
				else if (tokens_counter == 2) {
					command = tokens[0];
					port_data = tokens[1];	

//					printf("command = %s\n", command);
//					printf("port_data = %s\n", port_data);
				}

				//Bind socket for data connection
				printf("ftserver> Creating data socket...\n");

				if ((rv = getaddrinfo(NULL, port_data, &hints, &servinfo)) != 0) {
					fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
					return 1;
				}

				// loop through all the results and bind to the first we can
				for(p = servinfo; p != NULL; p = p->ai_next) {
					if ((sock_3 = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
						perror("server: socket");
						exit(1);
					}

					if (setsockopt(sock_3, SOL_SOCKET, SO_REUSEADDR, &yes,
						sizeof(int)) == -1) {
						perror("setsockopt");
						exit(1);
					}

					if (bind(sock_3, p->ai_addr, p->ai_addrlen) == -1) {
						close(sock_3);
						perror("server: bind");
						printf("Try different a different port number\n");
						exit(1);
					}
					break;
				}
				printf("ftserver> Data socket successfully bound...\n");

				//Listen for data transfer connection
				if (listen(sock_3, BACKLOG) == -1) {
					perror("listen");
					exit(1);
				}
				printf("ftserver> Listening on data socket on port %s...\n", port_data);

				//Send success to client for data connection handshake
				memset(message, 0, sizeof(message));
				strncpy(message, "success", 7);
				if ((send(sock_2, message, strlen(message), 0)) == -1) {
				    perror("Handshake message to client failed");
				    exit(1);
				}
				
				//Accept data transfer connection
				sin_size = sizeof their_addr;
				sock_3 = accept(sock_3, (struct sockaddr *)&their_addr, &sin_size);
				if (sock_3 == -1) {
					perror("accept");
					continue;
				}
				printf("ftserver> Data connection accepted\n");

				//Provide file info to client on data connection
				if (strncmp (command, "-l", 2) == 0) {		//If command = -l get directory listing
					int dir_counter = 0;
					int dir_size = 0;	//in bytes
					char * dir_listing[10] = {0};	
					DIR *dir;
					struct dirent *ep;
					dir = opendir ("./");

					if (dir != NULL) {	//iterate through directory
						while ((ep = readdir (dir)) != NULL) {
							if (ep->d_name[0] != '.') {		//ignore hidden files
//								printf("%s\n", ep->d_name);
								dir_listing[dir_counter] = (char *) malloc(strlen(ep->d_name) + 1);
								strcpy(dir_listing[dir_counter], ep->d_name);
								dir_size += strlen( dir_listing[dir_counter]);
//								printf("%s\n", dir_listing[dir_counter]);
//								printf("size = %i\n", dir_size);
								dir_counter++;								
							}
						}
						(void) closedir (dir);
					}	
					else {
						perror ("Couldn't opent directory");
					}	
	
					//Send directory listing to client
					char buffer_dir[dir_size + (dir_counter - 1)];	//bytes + spaces to seperate words for tokenization
//					printf(">>>>>> %ld\n", sizeof(buffer_dir));
					memset(buffer_dir, 0, sizeof (buffer_dir));

					for (int i = 0; i < dir_counter; i++) {		//add dir listing to buffer
						strcat (buffer_dir, dir_listing[i]);	
						strcat (buffer_dir, " " );	
//						puts (buffer_dir);
					}	

					printf("ftserver> Sending directory listing to client...\n");
					if ((send(sock_3, buffer_dir, strlen(buffer_dir), 0)) == -1) {	//send directory listing on data connection
					    perror("Failed to send directory listing to client");
					    exit(1);
					}
/*
					//Free heap
					for (int i = 0; i < tokens_counter; i++) {
						free(tokens[i]);
					}

					for (int i = 0; i < dir_counter; i++) {
						free(dir_listing[dir_counter]);
					}
*/
				}
				//If command = -g, then transfer file to client
				else if (strncmp (command, "-g", 2) == 0) {		
					ifstream infile; 
					infile.open (file_name);
				
					//If file does not exist send error message to client	
					if (!infile.is_open()) {
						memset(message, 0, sizeof(message));
						strncpy(message, "file_dne", 8);
						
						//Send DNE message
						if ((send(sock_2, message, strlen(message), 0)) == -1) {
						    perror("file_dne message failed");
						}
						perror("Can't open file");
					}
					//File exists
					else {
						//Send file size and filename to client
						infile.seekg (0, infile.end);
						int file_size = infile.tellg();
						infile.seekg (0, infile.beg);
//						printf("file_size = %i\n", file_size);

						//Prepare file size and name to send
						memset(message, 0, sizeof(message));
						sprintf (message, "%d", file_size);	
						message[strlen(message)] = ' ';
						strncat (message, file_name, strlen(file_name));
//						printf("File size & name = %s\n", message);
	
						printf("ftserver> Sending file size and name to client...\n");
						if ((send(sock_2, message, strlen(message), 0)) == -1) {
						    perror("Failed to send file size");
						}

						//Wait for client's receive message
						memset(buffer, 0, sizeof(buffer));
						if((read_size = recv(sock_2, buffer, 100, 0)) == 1) {
							perror("Could not receive file size confirmation");
						}
						else {
							//Send file contents to client
							char buffer_file[512] = {0};
//							char data[file_size] = {0};
							char * data;
							data = (char *) malloc (file_size);
							
							while (infile >> data) {
								strcat(buffer_file, data);
								buffer_file[strlen(buffer_file)] = ' ';
//								cout << data << ' ';
							}
							infile.close();					
//							printf("%s\n", buffer_file);

							printf("ftserver> Sending file content to client...\n");
							if ((send(sock_3, buffer_file, strlen(buffer_file), 0)) == -1) {
							    perror("Failed to send file contents.");
							}
						}
					}
				}

//				freeaddrinfo(servinfo); // all done with this structure


/*
				if (p == NULL) {
					fprintf(stderr, "server: failed to bind\n");
					exit(1);
				}
*/
			}
			close(sock_3);
			close(sock_2);
			exit(0);
		}
		close(sock_2); // parent doesn't need this
	}

	return 0;
}


/* ********** FUNCTION DEFINITIONS ********** */
void sigchld_handler(int s) {
	//waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}
	
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)	{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

