#include "recorder_server.h"

void init_server(){
	printf("Initializing the recorder server!\n");

	// Open up a socket
	recorder_socket = socket( AF_INET, SOCK_STREAM, NULL );

	// Bind socket
}
