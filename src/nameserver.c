#include "nameserver.h"

int main(){
	iplist = NULL;
	list_add(&iplist, "Kiran", "127.0.0.1", "1337", SOCK_STREAM);
	list_add(&iplist, "John", "127.0.0.2","1337", SOCK_STREAM);
	list_add(&iplist, "Cliff", "127.0.0.3", "1337", SOCK_STREAM);
	list_add(&iplist, "Batman", "127.0.0.4", "1337", SOCK_STREAM);
	list_print(iplist);


	init_server();
	message_listen();
	return 0;
}


void init_server(){

	// Open up a socket
	nameserver_socket = socket( AF_INET, SOCK_STREAM, 0 );
	if( nameserver_socket == -1 ){
		perror("socket");
		exit(1);
	}

	// Build sock addr
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port =  htons(NAMESERVER_LISTEN_PORT);

	int addr_size = sizeof(struct sockaddr_in);

	int optval = 1;
    if( (setsockopt(nameserver_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)) == -1){
        perror("setsockopt");
        exit(1);
    }

	// Bind socket
	if( bind( nameserver_socket, &addr, addr_size ) == -1) {
		perror("bind");
		exit(1);
	}

	// Listen on the socket for connections
	if( listen( nameserver_socket, BACKLOG ) == -1) {
		perror("listen");
		exit(1);
	}

	printf("[NAMESERVER] Server initialization complete.\n");

}

void message_listen(){
	struct sockaddr_storage their_addr;
	int addr_size = sizeof(struct sockaddr_storage);
	int acceptfd;
	while(1){
		memset(&their_addr, 0, sizeof(struct sockaddr_storage));

		printf("Waiting for a connection...\n");
		if(( acceptfd = accept( nameserver_socket, (struct sockaddr *)&their_addr, &addr_size )) == -1 ){
			perror("accept");
			exit(1);
		}
		printf("Connection recieved!\n");
		handle_connection(acceptfd);
	}
}

void handle_connection(int fd){
	char headerCode = 10; //Hardcode some value that isn't any of the defined properties
	if( read (fd, &headerCode, 1) != 1){
		perror("read");
		exit(1);
	}
	switch(headerCode){
		case FIND:
			printf("[NAMESERVER] FIND received!\n");
			break;
		case ADD:

			printf("[NAMESERVER] ADD received!\n");
			int size = 0;
			
			// Send size of message
			if( read( fd, &size, sizeof(int)) == -1){
				perror("read");
				exit(1);
			}
			printf("Incoming message size: %d bytes\n", size);
			char * msg = malloc(size);
			memset(msg, 0, size);

			if( read( fd, msg, size) == -1){
				perror("read");
				exit(1);
			}
			printf("[NAMESERVER] received msg: %s\n", msg);
			break;
		case EXIT:
			printf("[NAMESERVER] EXIT received!\n");
			break;
		default:
			printf("[NAMESERVER] Invalid header code\n");
			break;
	}
}
