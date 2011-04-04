#include "recorder_server.h"

int seconds_elapsed;

int init_connection( int port, int protocol ){
	// Open up a socket
	int conn_socket = socket( AF_INET, protocol, 0 );
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

	if(protocol == SOCK_STREAM)
	{
		// Listen on the socket for connections
		if( listen( conn_socket, BACKLOG ) == -1) {
			perror("listen");
			exit(1);
		}
	}
	else if(protocol == SOCK_DGRAM)
	{
		;//?
	}

  return conn_socket;
}

void* calculate_stats(){
	printf("[RECORER] Starting bandwidth stats\n");
  int current_bandwidth;
	while( stopRecording == 0)
  {
    seconds_elapsed++;
    pthread_mutex_lock(&bytes_sent_mutex);
    current_bandwidth = bytes_sent*8;
    printf("[%ds] Outgoing Bandwidth: %dbps\n", seconds_elapsed, current_bandwidth);
    bytes_sent = 0;
    sleep(1);
  }

	pthread_exit(NULL);

	/*timer_t timer_id;
	timer_create(CLOCK_REALTIME, NULL ,&timer_id);
	struct itimerspec val;
	memset(&val, 0, sizeof(struct itimerspec));
	val.it_value.tv_sec = 1;
	val.it_value.tv_nsec = 0;
	timer_settime(timer_id, NULL, &val, NULL);
  */
}


int accept_connection(int socket, int protocol){
	int fd;
	if(protocol == SOCK_STREAM)
	{
		socklen_t addr_size = (socklen_t)sizeof(struct sockaddr_storage);
		
		struct sockaddr_storage their_addr;
		memset(&their_addr, 0, sizeof(struct sockaddr_storage));
		fd = accept( socket, (struct sockaddr *)&their_addr, &addr_size );
		if(fd == -1 ){
			perror("accept connection");
			exit(1);
		}
	}
	else if(protocol == SOCK_DGRAM)
	{
		;//?
	}
	return fd;
}

void establish_control_connection(){
	recorder_control_socket = init_connection(CONTROL_PORT, SOCK_STREAM);
	printf("Waiting for a peer connection...\n");
}

void establish_video_connection(){
	recorder_video_socket = init_connection(VIDEO_PORT, av_protocol);
}

void establish_audio_connection(){
	recorder_audio_socket = init_connection(AUDIO_PORT, av_protocol);
}


void send_init_control_packet( AVStream* stream0, AVStream* stream1 ) {
  AVStream* audio_stream;
  AVStream* video_stream;
  if( stream0->codec->codec_type == AVMEDIA_TYPE_AUDIO )
  {
    audio_stream = stream0;
    video_stream = stream1;
  }
  else
  {
    audio_stream = stream1;
    video_stream = stream0;
  }

  control_packet cp;
  cp.audio_codec = *audio_stream->codec->codec;
  cp.video_codec = *video_stream->codec->codec;
  cp.audio_codec_ctx = *audio_stream->codec;
  cp.video_codec_ctx = *video_stream->codec;
  HTTP_packet* np = control_to_network_packet(&cp);
  printf("TYPE: %c\n", get_packet_type(np));
  xwrite(controlfd, np );

  // Keep track of bandwidth
  bytes_sent += np->length;
}

void establish_peer_connections(int protocol){
	av_protocol = protocol;
	establish_control_connection();
	establish_video_connection();
	establish_audio_connection();

	controlfd = accept_connection(recorder_control_socket, SOCK_STREAM);
	videofd = accept_connection(recorder_video_socket, av_protocol);
	audiofd = accept_connection(recorder_audio_socket, av_protocol);
	
	/*
	struct timeb tp; 
	int t1, t2;
	printf("Connection recieved!\n");
	char * buf = malloc(25);
	memset(buf, 0, 25);
	strcpy(buf, "Hello World!\0");

	ftime(&tp);
	t1 = (tp.time * 1000) + tp.millitm;
	//printf("Start: %d\n", tp.millitm);
	
	// Send the packet to the client
	if( write(controlfd, buf, 25) == -1){
		perror("write");
		exit(1);
	}

	// Get the time elapsed on the client
	int * t3 = malloc(sizeof(int));
	if( read(controlfd, t3, sizeof(int)) == -1 ){
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

void stream_video_packets(){
	while(stopRecording == 0){
		video_frame_write();
	}
	pthread_exit(NULL);
}

void stream_audio_packets(){
	while(stopRecording == 0){
		audio_segment_write();
	}
	pthread_exit(NULL);
}

void listen_control_packets(){
	//listen for control and pantilt packets.
	void* buffer = malloc(100);
	while(stopRecording == 0){
		int size = read(controlfd, buffer, 100);
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
				printf("Pan/tilt packet received\n");
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
	pthread_exit(NULL);
}

//______________NAME SERVER_______________

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
