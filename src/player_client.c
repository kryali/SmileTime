#include "player_client.h"

void client_init(){

    struct addrinfo hints2, * res2;
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints2.ai_socktype = SOCK_STREAM;
    hints2.ai_flags = AI_PASSIVE | AI_NUMERICSERV;     // fill in my IP for me
	//char * hostname = "localhost";
	char * hostname = "127.0.0.1";
    char * port = "1336";

    if((getaddrinfo(hostname, port, &hints2, &res2)) != 0){
        perror("getaddrinfo");
        exit(1);
    }

	if( (res2->ai_protocol & SOCK_NONBLOCK) == 0 ){
		printf("Socket is blocking\n");
	}
 	if( (player_socket = socket(res2->ai_family, res2->ai_socktype, res2->ai_protocol)) == -1){
		perror("socket");
		exit(1);
	}

	printf("Client connecting to server...\n");
	if( connect(player_socket, res2->ai_addr, res2->ai_addrlen) == -1 ){
			perror("connect");
			exit(1);
	}

    printf("Conected client \n");
	char * buf = malloc( 500 );
	memset(buf, 0 , 500);

	int dataread = 0;
	if( ( dataread = read(player_socket, buf, 25)) == -1){
		perror("read");
		exit(1);
	}
	printf("Received %d bytes: %s\n", dataread, buf);
}
