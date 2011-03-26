#include "recorder_server.h"

void init_server(){
	printf("Initializing the recorder server!\n");

	// Open up a socket
	recorder_socket = socket( AF_INET, SOCK_STREAM, NULL );
	if( recorder_socket == -1 ){
		perror("socket");
		exit(1);
	}

	// Build sock addr
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port =  htons(LISTEN_PORT);

	// Bind socket
	if( bind( recorder_socket, &addr, sizeof(addr) ) == -1) {
		perror("bind");
		exit(1);
	}

	// Listen on the socket for connections
	if( listen( recorder_socket, BACKLOG ) == -1) {
		perror("listen");
		exit(1);
	}
}
