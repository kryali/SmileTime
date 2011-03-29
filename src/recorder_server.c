#include "recorder_server.h"

void init_server(){
	printf("Initializing the recorder server!\n");
	init_control_connection();
}

void init_control_connection(){
	// Open up a socket
	recorder_socket = socket( AF_INET, SOCK_STREAM, 0 );
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

	int addr_size = sizeof(struct sockaddr_in);

	int optval = 1;
    if( (setsockopt(recorder_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)) == -1){
        perror("setsockopt");
        exit(1);
    }

	// Bind socket
	if( bind( recorder_socket, &addr, addr_size ) == -1) {
		perror("bind");
		exit(1);
	}

	// Listen on the socket for connections
	if( listen( recorder_socket, BACKLOG ) == -1) {
		perror("listen");
		exit(1);
	}

	struct timeb tp; 
	int t1, t2;

		//	addr_size = sizeof(struct sockaddr_storage);
		struct sockaddr_storage their_addr;
		memset(&their_addr, 0, sizeof(struct sockaddr_storage));

		printf("Waiting for a connection...\n");
		if(( acceptfd = accept( recorder_socket, (struct sockaddr *)&their_addr, &addr_size )) == -1 ){
			perror("accept");
			exit(1);
		}/*
		printf("Connection recieved!\n");
		char * buf = malloc(25);
		memset(buf, 0, 25);
		strcpy(buf, "Hello World!\0");

		ftime(&tp);
		t1 = (tp.time * 1000) + tp.millitm;
		//printf("Start: %d\n", tp.millitm);
		
		// Send the packet to the client
		if( write(acceptfd, buf, 25) == -1){
			perror("write");
			exit(1);
		}

		// Get the time elapsed on the client
		int * t3 = malloc(sizeof(int));
		if( read(acceptfd, t3, sizeof(int)) == -1 ){
			perror("read");
			exit(1);
		}
		//printf("Elapsed time from client: %d\n", *t3);

		ftime(&tp);
		t2 = (tp.time * 1000) + tp.millitm;
		//printf("End: %d\n", tp.millitm);

		printf("Message Sent!\n");
		printf("RTT:%ds\n", t2-t1-*t3);
*/
	//listen for control and pantilt packets.
	void* buffer = malloc(100);
	while(1){
		int size = read(acceptfd, buffer, 100);
		if( size == -1 ){
			perror("read");
			exit(1);
		}
		printf("read packet of size: %d\n",size);
		HTTP_packet packet;
		packet.message = buffer;
		packet.length = size;
		char packet_type = get_packet_type(&packet);
		switch(packet_type)
		{
			case CONTROL_PACKET:
				printf("received control packet\n");
			break;
			case PANTILT_PACKET:
				pantilt_packet* pt = to_pantilt_packet(&packet);
				if(pt->type == PAN)
					printf("replace this line with a pan of distance %d\n", pt->distance);
				if(pt->type == TILT)
					printf("replace this line with a tilt of distance %d\n", pt->distance);
			break;
			default:
				printf("received INVALID packet\n");
			break;
		}
	}
	free(buffer);
}

