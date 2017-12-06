//NAME: Dawei Huang
//EMAIL: daweihuang@g.ucla.edu
//ID: 304792166

#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <arpa/inet.h> 
#include <unistd.h>    
#include <termios.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <mcrypt.h>
#include <signal.h>
#include <sys/wait.h>

int socket_desc;
int new_socket_desc;
int socket_len;

//encryption data structures
char* key;
int file_size;
MCRYPT encryption_obj, decryption_obj;

//options flags
bool port_flag = false;
bool encrypt_flag = false;

//option parameters
int port_number;
char* encrypt_fn; 

//termios
struct termios oldtio, newtio;

//socket address
struct sockaddr_in server_address, client_address;

int ARRAY_LENGTH = 256;
int toshell[2];
int fromshell[2];
bool instaKill = false;
const int OUTPUT = 0;
const int INPUT = 1;
int pid;

void process_args(int argc, char** argv);
void setUpTermios(struct termios* oldtio, struct termios* newtio);
void establishSockets(struct sockaddr_in* server_address, struct sockaddr_in* client_address);
void doShellStuff();
void create_pipe(int pipefd[2]);
void read_and_write_shell(int fd_output);
void restoreTermios();
void check_read (int read_bytes);
void signal_handler(int signo);
void outputStatus();
char* read_key(char* filename);
void doEncryptionStuff();
void encrypt_ (char* c);
void decrypt_ (char* c);
void undoEncryptionStuff();
void close_sockets();
void close_pipes();

void end_program() {
	if (instaKill == true) {
		close(toshell[INPUT]);
		outputStatus();
		close_pipes();
		close_sockets();
	}
	else {
		kill(pid, SIGKILL);
		outputStatus();
		close_pipes();
		close_sockets();
	}
}

int main (int argc, char** argv) {
	process_args(argc, argv);
	
	establishSockets(&server_address, &client_address);
	atexit(end_program);
	signal(SIGCHLD, signal_handler);
	doShellStuff();
	read_and_write_shell(new_socket_desc);	

	if (encrypt_flag == true) {undoEncryptionStuff();}
	exit(0);
}

void process_args(int argc, char** argv) {
		
	//sets up the --shell option
	int opt;
	const char* SHORT_OPTS = "p:e";
	const struct option LONG_OPTS[] = {
            {"port", 1, NULL, 'p'},
            {"encrypt", 1, NULL, 'e'},
			{NULL, 0, NULL, 0}
	};
	
	while ((opt = getopt_long(argc, argv, SHORT_OPTS, LONG_OPTS, NULL)) != -1) {
		switch (opt) {
			case 'p':
				port_flag = true;
				port_number = atoi(optarg);
				break;
			case 'e':
				encrypt_flag = true;
				encrypt_fn = optarg;
				key = read_key(encrypt_fn);
				doEncryptionStuff();
				break;
			default:
				fprintf(stderr, "unrecognized argument: %d \n", opt);
				fprintf(stderr, "proper argument is --port=number --log=filename --encrypt=filename.");
				exit(1);
				break;
		}
	}
	
	if (port_flag != true) {
		fprintf(stderr, "Please insert a port number for your --port option.");
		exit(1);
	}
}

void establishSockets(struct sockaddr_in* server_address, struct sockaddr_in* client_address) {
	
	//establish socket descriptor
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        fprintf(stderr, "Cannot create socket descriptor");
		exit(1);
    }
	
	//get server of localhost
	/*if ((*server = gethostbyname("localhost")) == NULL) {
		fprintf(stderr, "Cannot find host");
		exit(1);
	}*/
	
	//clean out server_address
	memset((char*) server_address, 0, sizeof(server_address));
	//prepare sockaddr_in structure
    server_address->sin_family = AF_INET;
	server_address->sin_addr.s_addr = INADDR_ANY;
	server_address->sin_port = htons(port_number);
 
	//Bind socket to port
	if( bind(socket_desc,(struct sockaddr *)server_address , sizeof(*server_address)) == -1) {
		fprintf(stderr, "failed to bind sockets");
		exit(1);
	}
	
	//listening for a connection
	listen(socket_desc , 5);
	
	//Accept and incoming connection
    socket_len = sizeof(*client_address);
    new_socket_desc = accept(socket_desc, (struct sockaddr *)client_address, (socklen_t*)&socket_len);
    if (new_socket_desc == -1)
    {
        fprintf(stderr, "failed to accept socket");
		exit(1);
    }	
}

void doShellStuff() {
	
	//create pipe
	create_pipe(toshell);
	create_pipe(fromshell);
	
	pid = fork();
	if (pid == -1) {
		fprintf(stderr, "failure to fork process.");
		exit(1);
	}
	else if (pid == 0) {
		//child process
		char* shell_arguments[2];
		shell_arguments[0] = (char*)malloc(9*sizeof(char));
		shell_arguments[0] = "/bin/bash";
		shell_arguments[1] = NULL;
		
		close(toshell[INPUT]);
		close(fromshell[OUTPUT]);
		dup2(toshell[OUTPUT], STDIN_FILENO);
		dup2(fromshell[INPUT], STDOUT_FILENO);
		dup2(fromshell[INPUT], STDERR_FILENO);
		close(toshell[OUTPUT]);
		close(fromshell[INPUT]);

		
		if (execv(shell_arguments[0], shell_arguments) == -1) {
			fprintf(stderr, "exec error.");
			exit(1);
		}
	}
	else {
	}
}

void create_pipe(int pipefd[2]) {
	if (pipe(pipefd) == -1) {
		fprintf(stderr, "Cannot create pipe\n");
		exit(1);
	}
	else {
		//fprintf(stderr, "Pipe works successfully\r\n");
	}
}

void close_pipes(){
	
	close(toshell[INPUT]);
	close(toshell[OUTPUT]);
	close(fromshell[INPUT]);
	close(fromshell[OUTPUT]);
}

void close_sockets() {
	
	close(socket_desc);
	close(new_socket_desc);
}

void read_and_write_shell(int fd_output) {
	struct pollfd p[2];
	
	//keyboard
	p[0].fd = fd_output	;
	p[0].events = POLLIN | POLLERR | POLLHUP;
	
	//shell output
	p[1].fd = fromshell[OUTPUT];
	p[1].events = POLLIN | POLLERR | POLLHUP;

	while (1) {
		
		int ret = poll(p, 2, 0);
		
		if (ret == -1) {
			fprintf(stderr, "polling error\n");
			exit(1);
		}
		
		char c;
		//this sends information from the socket
		//to the shell
		if (p[0].revents & POLLIN) {
			ssize_t read_bytes = read(p[0].fd, &c, 1);
			check_read(read_bytes);
			
			if (read_bytes != -1) {
				
				if (encrypt_flag == true) { 
					//fprintf(stderr, "encrypted: %c\n", c);
					decrypt_(&c); 
				}
				
				ssize_t write_bytes;
				//fprintf(stderr, "socket to shell: %d---%c\n", (int)c, c);
				if (c == '\r' || c == '\n') {
					char lf[1] = {'\n'};
					write_bytes = write(toshell[INPUT], lf, 1);
				} 
				else if (c == 0x04) { 
					//fprintf(stderr, "send char to shell: %d\n", (int)c);
					instaKill = true;
					exit(0);
				}
				else if (c == 0x03) {
					exit(0);
					
				}
				//for sending regular information
				else {
					
					write_bytes = write(toshell[INPUT], &c, 1);
				}
				
				if (write_bytes == -1) {
					fprintf( stderr , "cannot write bytes into stdout. 0000");
				}
			}
			else {
				fprintf(stderr, "cannot read bytes from keyboard.");
			}
			
		}
		
		//listening from shell to sockets
		if(p[1].revents & POLLIN) {
			ssize_t read_bytes = read(p[1].fd, &c, 1);
			check_read(read_bytes);
			
			if (read_bytes != 0) {
				
				//fprintf(stderr, "shell to socket: %d---%c\n",(int)c, c);
				
				if (encrypt_flag == true) { 
					encrypt_(&c); 
				} 
				
				ssize_t write_bytes;
				write_bytes = write(fd_output, &c, 1);	
				
				if (write_bytes == -1) {
					fprintf( stderr , "cannot write bytes into stdout. 0000");
				}
			}
			else {
				fprintf(stderr, "cannot read bytes from shell pipe.");
			}
			
		}
		
		if (p[1].revents & (POLLHUP | POLLERR)) {
			return;
		}
	}
}

//this was used to keep track of read_bytes
//to see that the program is still capable of
//reading bytes
void check_read (int read_bytes) {
	if (read_bytes == 0) {
		//fprintf(stderr, "goodbye");
		exit(0);
	}
	else if (read_bytes == -1) {
		fprintf (stderr, "Error in reading stdin.");
		exit(1);
	}
	else {
		//fprintf(stderr, "# of read_bytes: %d\n", read_bytes);
	}
}

void signal_handler(int signo) {
	if (signo == SIGINT || signo == SIGKILL) {
		//fprintf(stderr, "\n please exit.\n");
		exit(0);
    }
    if (signo == SIGPIPE) {
		outputStatus();
        exit(0);
    }
	
	if(signo == SIGCHLD ) {
		instaKill = true;
		exit(0);
	}
}

void outputStatus() {
	
	int status = 0;
	int signal = -1;
	int exit_status = -1;
	//fprintf(stderr, "pid----%d\n", pid);
	int wpid = waitpid(pid, &status,0);
	if(wpid == -1) {
		fprintf(stderr, "waitpid error.");
		exit(1);
	}
	
	if (WIFEXITED(status)){
		exit_status = WEXITSTATUS(status);
		signal = WTERMSIG(status);
	}
	//if (instaKill == true) {
		fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signal, exit_status);
	//}
	
}

//this reads the key from the text files
char* read_key(char* filename) {
	
	//used to provide information about the file of the key
	struct stat file_info;
	int file_des = open(filename, O_RDONLY);
	if(fstat(file_des, &file_info) == -1) { 
		fprintf(stderr, "failed to examine file stats"); 
		exit(1); 
		
		
		
		//
	}
	
	//allocate memory for file
	file_size = file_info.st_size;
	char* buffer = (char*) malloc(file_size);
	
	//read file
	if(read(file_des, buffer, file_size) == -1) { 
		fprintf(stderr, "failed to read key-file"); 
		exit(1); 
	}
	
	return buffer;
}

//this opens the mcrypt library and stores it
//in two MCRYPT objects, one for encryption
//and another one for decryptions
void doEncryptionStuff() {
	
	encryption_obj = mcrypt_module_open("blowfish", NULL, "cfb", NULL);
	
	 if (encryption_obj == MCRYPT_FAILED) {
        fprintf(stderr, "Cannot open mcrypt library1\n");
        exit(1);
    }
	
	if (mcrypt_generic_init( encryption_obj, key, file_size, NULL) == -1) {
		fprintf(stderr, "Cannot initialize encryption");
		exit(1);
	}
	
	decryption_obj = mcrypt_module_open("blowfish", NULL, "cfb", NULL);
	
	if (decryption_obj == MCRYPT_FAILED) {
        fprintf(stderr, "Cannot open mcrypt library2\n");
        exit(1);
    }
	
	if (mcrypt_generic_init(decryption_obj, key, file_size, NULL) == -1) {
		fprintf(stderr, "Cannot initialize decryption");
		exit(1);
	}
	
}

void encrypt_ (char* c) {
	if(mcrypt_generic(encryption_obj, c, 1) != 0) {
        fprintf(stderr, "fail to encrypt character\n");
        exit(1);
    }
}

	
void decrypt_ (char* c) {	
	if(mdecrypt_generic(decryption_obj, c, 1) != 0) {
        fprintf(stderr, "fail to decrypt character\n");
        exit(1);
    }
}

//this deinitializes the MCRYPT objects and closes the
//libraries
void undoEncryptionStuff() {
    mcrypt_generic_deinit(encryption_obj);
    mcrypt_module_close(encryption_obj);

    mcrypt_generic_deinit(decryption_obj);
    mcrypt_module_close(decryption_obj);
}