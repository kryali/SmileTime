#include "recorder_server.h"

int mobile_latency_packet_consumed;

void listen_peer_connections( int port){
	recorder_control_socket = listen_on_port(port, SOCK_STREAM);
	numPeers = 0;

	peer_fd = malloc(MAX_PEERS * sizeof(int));
	peer_info = malloc(MAX_PEERS * sizeof(struct sockaddr_in));
	int i = 0;
	for(; i < MAX_PEERS; i++){
		peer_fd[i] = -1;
		memset(&peer_info[i], 0, sizeof(struct sockaddr_in));
	}
}

int listen_on_port( int port, int protocol){
	// Open up a socket
	int conn_socket = socket( AF_INET, SOCK_STREAM, 0 );
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
		perror("tcp bind");
		exit(1);
	}

	// Listen on the socket for connections
	if( listen( conn_socket, BACKLOG ) == -1) {
		perror("listen");
		exit(1);
	}

  return conn_socket;
}

void init_udp_av(){
	int slen=sizeof(si_me);

	if ((video_socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		perror("socket");

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(VIDEO_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(video_socket, &si_me, sizeof(si_me))==-1)
		perror("udp bind");
	printf("[VIDEO] UDP Socket is bound\n");
	jpgBuffer = malloc(UDP_MAX);
	memset(jpgBuffer, 0, UDP_MAX);
}

void accept_peer_connection(int socket, int protocol){
	accept_connection(socket, numPeers, protocol);
	printf("%s\n",inet_ntoa(peer_info[numPeers].sin_addr));
	numPeers++;
}

void accept_connection(int socket, int peerIndex, int protocol){
	int fd;
	int addr_size = sizeof(struct sockaddr_in);
	struct sockaddr_in their_addr;
	memset(&their_addr, 0, sizeof(struct sockaddr_in));
	fd = accept( socket, (struct sockaddr *)&their_addr, &addr_size );
	if(fd == -1 ){
		perror("accept connection");
		exit(1);
	}
	their_addr.sin_port = htons(AV_PORT);
	peer_fd[peerIndex] = fd;
	peer_info[peerIndex] = their_addr;
}

void* calculate_stats(){
	printf("[smiletime] Starting bandwidth stats\n");
	int sent_bandwidth;
	int received_bandwidth;
	bytes_sent = 0;
	bytes_received = 0;
	seconds_elapsed = 0;
	while( stopRecording == 0)
	{
		if(streaming == 1){
			seconds_elapsed++;
			//pthread_mutex_lock(&bytes_sent_mutex);
			//pthread_mutex_unlock(&bytes_sent_mutex);
			sent_bandwidth = bytes_sent*8;
			received_bandwidth = bytes_received*8;
			printf("[%ds] Outgoing Bandwidth: %dbps\n", seconds_elapsed, sent_bandwidth);
			printf("[%ds] Incoming Bandwidth: %dbps\n", seconds_elapsed, received_bandwidth);
			bytes_sent = 0;
			bytes_received = 0;
			sleep(1);
		}
	}
	pthread_exit(NULL);
}

void* sendLatencyPackets(){
  struct timeb t;
	printf("[smiletime] Starting latency monitoring\n");

	while( stopRecording == 0)
	{
		if(streaming == 1){
      // Get current time
      ftime(&t);
      
      // Create the latency packet
      latency_packet l;
      l.packetType = LATENCY_PACKET;
      l.peer_sender = 0; // This latency packet is for Desktop-to-Mobile latency
      l.time_sent = t.time*1000 + t.millitm;
      HTTP_packet* latencypacket = create_HTTP_packet(sizeof(latency_packet));
      memcpy(latencypacket->message, &l, sizeof(latency_packet));

      // Send the latency packet to all peers
      ywrite(latencypacket);
      destroy_HTTP_packet(latencypacket);

      printf("[smiletime] Sent latency packet\n");

			sleep(10);
		}
	}
	pthread_exit(NULL);
}

void calculate_latency( latency_packet *l ){
  unsigned long dtom_latency;
  unsigned long mtod_latency;
  struct timeb now;

  if( l->peer_sender < 0 )
    return;

  // Get current time
  ftime(&now);

  // Desktop-to-Mobile latency
  if( l->peer_sender == 0 )
  {
    // Latency is roundtrip / 2
    dtom_latency = ( (now.time*1000 + now.millitm) - l->time_sent ) / 2;

    // Print the Desktop-to-Mobile latency
    printf("[%ds] Desktop-to-Mobile Latency: %lums\n", seconds_elapsed, dtom_latency);
  }
  else
  {
    if( mobile_latency_packet_consumed == 0 )
    {
      // Send the latency packet back to the mobile
      HTTP_packet* latencypacket = create_HTTP_packet(sizeof(latency_packet));
      memcpy(latencypacket->message, l, sizeof(latency_packet));
      ywrite(latencypacket);
      destroy_HTTP_packet(latencypacket);

      mobile_latency_packet_consumed = 1;
    }
    else
    {
      // Print the Mobile-to-Desktop latency
      printf("[%ds] Mobile-to-Desktop Latency: %lums\n", seconds_elapsed, l->time_sent);
      mobile_latency_packet_consumed = 0;
    }

    //mtod_latency = ( (now.time*1000 + now.millitm) - l->time_sent - 15000 );
    //while( mtod_latency < 0 ) mtod_latency += 10; // lulz?
  }
}

void send_text_message(char* str){
	text_packet txt;
	txt.packetType = TEXT_PACKET;
  memset(txt.message,0,TEXT_MAX_SIZE);
	strcpy(txt.message, str);
	printf("sending text message: %s\n", str);
	HTTP_packet* txtpacket = create_HTTP_packet(sizeof(text_packet));
	memcpy(txtpacket->message, &txt, sizeof(text_packet));
	ywrite(txtpacket);
	destroy_HTTP_packet(txtpacket);
}

void listen_control_packets(){
	//listen for control and pantilt packets.
	int packetType;
	HTTP_packet* packet;
	int i;
	struct timeval timeout;
   timeout.tv_sec = 2; // 2 second timeout.  New users will be able to send new messages within 2 seconds.
   timeout.tv_usec = 0;

	while(stopRecording == 0){
		FD_ZERO(&fds);
		nfds = 0;
		for(i = 0; i < numPeers; i++){
			if(peer_fd[i] != -1){
				if(peer_fd[i] > nfds) nfds = peer_fd[i];
				FD_SET(peer_fd[i], &fds);
			}
		}
		if(select(nfds+1, &fds, NULL, NULL, &timeout) > 0)
		{
			for(i = 0; i < numPeers; i++){
				if(peer_fd[i] != -1 && FD_ISSET(peer_fd[i], &fds)){
					int size = recv(peer_fd[i], &packetType, sizeof(packetType), MSG_PEEK);
					if( size == -1 || size == 0 ){
						peer_fd[i] = -1;
						break;
					}
					switch(packetType)
					{
						case CONTROL_PACKET:
							packet = create_HTTP_packet(sizeof(control_packet));
							yread(packet, peer_fd[i]);
						break;
						case PANTILT_PACKET:
							packet = create_HTTP_packet(sizeof(pantilt_packet));
							yread(packet, peer_fd[i]);
							pantilt_packet* pt = to_pantilt_packet(packet);
							printf("pan: %d, tilt: %d\n", pt->pan, pt->tilt);
							pan_relative(pt->pan);
							tilt_relative(pt->tilt);
							free(pt);
						break;
						case TEXT_PACKET:
							packet = create_HTTP_packet(sizeof(text_packet));
							yread(packet, peer_fd[i]);
							text_packet* tp = to_text_packet(packet);
							char username[8];	
							strcpy(username, "peer : ");
							username[4] = i + '0';
							tp->message[strlen(tp->message)] = '\n';
							println(username, 7, tp->message, strlen(tp->message));
							free(tp);
						break;
						case LATENCY_PACKET:
							packet = create_HTTP_packet(sizeof(latency_packet));
							yread(packet, peer_fd[i]);
							latency_packet* lp = to_latency_packet(packet);
              calculate_latency(lp);
							free(lp);
						break;
						default:
							printf("received UNRECOGNIZED packet\n");
						break;
					}
					destroy_HTTP_packet(packet);
					packet = NULL;
				}
			}
		}
	}
	pthread_exit(NULL);
}

//______________NAME SERVER_______________

void register_nameserver(char * name, char * protocol, char * control_port){
	printf("[smiletime] registering IP with nameserver\n");

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

	
	printf("[smiletime] connecting to nameserver...\n");
	if( connect(nameserver_socket, res2->ai_addr, res2->ai_addrlen) == -1 ){
		perror("nameserver connect");
		exit(1);
	}
	printf("[smiletime] nameserver connected!...\n");

	char headerCode = ADD;
	if( write( nameserver_socket, &headerCode, 1) == -1){
		perror("nameserver write1");
		exit(1);
	}

	//char * name = "kiran";
	char * ip = getIP();
	int size = 0;
	char * msg = nameServerMsg(name, ip, control_port, protocol, &size);
	printf("Sending message: %s of length %d\n", msg, size);
	
	// Send size );of message
	if( write( nameserver_socket, &size, sizeof(int)) == -1){
		perror("nameserver write2");
		exit(1);
	}

	if( write( nameserver_socket, msg, size) == -1){
		perror("nameserver write3");
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
