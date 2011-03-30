#include "recorder_server.h"

void init_server(){
	printf("Initializing the recorder server!\n");

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

	register_nameserver();
	listen_connections();
}

void register_nameserver(){
	printf("[Recorder] registering IP with nameserver\n");

	// Connect to nameserver
    struct addrinfo hints2, * res2;
	int nameserver_socket;
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints2.ai_socktype = SOCK_STREAM;
    hints2.ai_flags = AI_PASSIVE | AI_NUMERICSERV;     // fill in my IP for me
	char * hostname = "127.0.0.1";
//    char * port = NAMESERVER_LISTEN_PORT;
    char * port = "1337";
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

	char * msg = "kiran#127.0.0.1:1337#1";
	printf("IP ADD: %s\n", getIP());
	int size = strlen(msg);
	
	// Send size of message
	if( write( nameserver_socket, &size, sizeof(int)) == -1){
		perror("write");
		exit(1);
	}

	if( write( nameserver_socket, msg, strlen(msg)) == -1){
		perror("write");
		exit(1);
	}
}

void listen_connections(){

	struct timeb tp; 
	int t1, t2;
	int addr_size = sizeof(struct sockaddr_storage);

	while(1){
		//	addr_size = sizeof(struct sockaddr_storage);
		struct sockaddr_storage their_addr;
		memset(&their_addr, 0, sizeof(struct sockaddr_storage));

		printf("Waiting for a connection...\n");
		if(( acceptfd = accept( recorder_socket, (struct sockaddr *)&their_addr, &addr_size )) == -1 ){
			perror("accept");
			exit(1);
		}
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
	}
}
char *  getIP() {
    struct ifaddrs * ifAddrStruct=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct!=NULL) {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if( strcmp( ifAddrStruct->ifa_name, "eth1") == 0 ){
				return addressBuffer;
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
