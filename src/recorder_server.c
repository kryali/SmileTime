#include "recorder_server.h"

void init_server(char prot){
	printf("Initializing the recorder server!\n");
	av_protocol = prot;
	init_control_connection();
	init_video_connection();
	init_audio_connection();
}

int init_connection( int port ){
	// Open up a socket
	int conn_socket = socket( AF_INET, SOCK_DGRAM, 0 ); //change based on connection
	if( conn_socket == -1 ){
		perror("socket");
		exit(1);
	}

	// Build sock addr
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port =  htons(port);

	int addr_size = sizeof(struct sockaddr_in);

	int optval = 1;
    if( (setsockopt(conn_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)) == -1){
        perror("setsockopt");
        exit(1);
    }

	// Bind socket
	if( bind( conn_socket, &addr, addr_size ) == -1) {
		perror("bind");
		exit(1);
	}

	if(av_protocol == TCP)
	{
		// Listen on the socket for connections
		if( listen( conn_socket, BACKLOG ) == -1) {
			perror("listen");
			exit(1);
		}
	}
	if(av_protocol == UDP)
	{
		;//?
	}

  return conn_socket;
}

void init_video_connection(){
  recorder_video_socket = init_connection(VIDEO_PORT);
}

void init_audio_connection(){
  recorder_audio_socket = init_connection(AUDIO_PORT);
}

void init_control_connection(){
	// Open up a socket
	recorder_control_socket = socket( AF_INET, SOCK_STREAM, 0 );
	if( recorder_control_socket == -1 ){
		perror("socket");
		exit(1);
	}

	// Build sock addr
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port =  htons(CONTROL_PORT);

	int addr_size = sizeof(struct sockaddr_in);

	int optval = 1;
    if( (setsockopt(recorder_control_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)) == -1){
        perror("setsockopt");
        exit(1);
    }

	// Bind socket
	if( bind( recorder_control_socket, &addr, addr_size ) == -1) {
		perror("bind");
		exit(1);
	}

	// Listen on the socket for connections
	if( listen( recorder_control_socket, BACKLOG ) == -1) {
		perror("listen");
		exit(1);
	}

}

void register_nameserver(char * name, char * protocol, char * control_port){
	printf("[RECORDER] registering IP with nameserver\n");

	// Connect to nameserver
    struct addrinfo hints2, * res2;
	int nameserver_socket;
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints2.ai_socktype = SOCK_STREAM;
    hints2.ai_flags = AI_PASSIVE | AI_NUMERICSERV;     // fill in my IP for me

	char * hostname = NAMESERVER_IP;
    char * port = NAMESERVER_LISTEN_PORT_S;

    if((getaddrinfo(hostname, port, &hints2, &res2)) != 0){
        perror("getaddrinfo");
        exit(1);
    }

 	if( (nameserver_socket = socket(res2->ai_family, res2->ai_socktype, res2->ai_protocol)) == -1){
		perror("socket");
		exit(1);
	}

	
	printf("[RECORDER] connecting to nameserver...\n");
	if( connect(nameserver_socket, res2->ai_addr, res2->ai_addrlen) == -1 ){
		perror("connect");
		exit(1);
	}
	printf("[RECORDER] nameserver connected!...\n");

	char headerCode = ADD;
	if( write( nameserver_socket, &headerCode, 1) == -1){
		perror("write");
		exit(1);
	}

	//char * name = "kiran";
	char * ip = getIP();
	int size = 0;
	char * msg = nameServerMsg(name, ip, control_port, protocol, &size);
	printf("Sending message: %s of length %d\n", msg, size);
	
	// Send size );of message
	if( write( nameserver_socket, &size, sizeof(int)) == -1){
		perror("write");
		exit(1);
	}

	if( write( nameserver_socket, msg, size) == -1){
		perror("write");
		exit(1);
	}

	free(msg);
}

char * nameServerMsg(char * name, char * ip, char * control_port, char * protocol, int * size){
	*size = strlen(ip) + strlen(control_port) + 5 + strlen(name);
	char * msg = malloc(*size);
	memset(msg, 0, *size);
	strcat(msg, name);
	strcat(msg, "#");
	strcat(msg, ip);
	strcat(msg, ":");
	strcat(msg, control_port);
	strcat(msg, "#");
	strcat(msg, protocol);
	msg[*size-1] = '\0';
	return msg;
}

void establish_peer_connection(){
	struct timeb tp; 
	int t1, t2;
	int addr_size = sizeof(struct sockaddr_storage);

	//	addr_size = sizeof(struct sockaddr_storage);
	struct sockaddr_storage their_addr;
	memset(&their_addr, 0, sizeof(struct sockaddr_storage));

	printf("Waiting for a connection...\n");
	if(( acceptfd = accept( recorder_control_socket, (struct sockaddr *)&their_addr, &addr_size )) == -1 ){
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
}

void listen_control_packets(){
	//listen for control and pantilt packets.
	void* buffer = malloc(100);
	while(1){
		int size = read(acceptfd, buffer, 100);
		if( size == -1 || size == 0 ){
			perror("read");
			exit(1);
		}
		//printf("read packet of size: %d\n",size);
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
				;
				pantilt_packet* pt = to_pantilt_packet(&packet);
				if(pt->type == PAN)
					pan_relative(pt->distance);
				else if(pt->type == TILT)
					tilt_relative(pt->distance);
				break;
			default:
				printf("received INVALID packet\n");
				break;
		}
	}
	free(buffer);
}
char *  getIP() {
    struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct!=NULL) {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
//            char addressBuffer[INET_ADDRSTRLEN];
			char * addressBuffer = malloc(INET_ADDRSTRLEN);
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer); 
			if( strcmp( ifAddrStruct->ifa_name, "eth1") == 0 || strcmp( ifAddrStruct->ifa_name, "eth0") == 0  ){
				return (char *)addressBuffer;
			}
        } else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer); 
        } 
        ifAddrStruct=ifAddrStruct->ifa_next;
    }
    return "(null)";
}
