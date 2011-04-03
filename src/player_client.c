#include "player_client.h"
#include "player.h"
#include "video_play.h"
#include "player.h"

extern int bytes_received;
extern pthread_mutex_t bytes_received_mutex;


extern VideoState * global_video_state;

void establish_video_connection(){
	printf("[PLAYER] Connecting video socket\n");
  player_video_socket = socket_connect(hostname, VIDEO_PORT_S, protocol);
}

void establish_audio_connection(){
	printf("[PLAYER] Connecting audio socket\n");
  player_audio_socket = socket_connect(hostname, AUDIO_PORT_S, protocol);
}

void establish_control_connection(){
	printf("[PLAYER] Connecting control socket\n");
  player_control_socket = socket_connect(hostname, CONTROL_PORT_S, SOCK_STREAM);
}

void init_gis(VideoState * global_video_state_in) {
  global_video_state = global_video_state_in;
//  printf("GIS: audio buf size: %d\n", global_video_state->audio_buf_size);
//  printf("GIS: PacketQueue size:%d\n", global_video_state->videoq.size);
//  printf("GIS: parse thread id:%d\n", global_video_state->parse_tid);
//  printf("GIS: video thread id:%d\n", global_video_state->parse_tid);
}

void establish_peer_connections(){
  printf("[PLAYER] Establishing peer connections\n");
  establish_control_connection();
  establish_video_connection();
  establish_audio_connection();

  listen_packets();
}

void listen_packets(){
  printf("[PLAYER] Launching listen threads\n");
//  pthread_create(&video_thread_id, NULL, listen_video_packets, NULL);
  pthread_create(&audio_thread_id, NULL, listen_audio_packets, NULL);
  pthread_create(&control_thread_id, NULL, listen_control_packets, NULL);
}

void * listen_audio_packets(){
  while(1){
      av_packet *packet = read_av_packet(player_audio_socket);
	// This returns av_packet but this is AVPacket?
	  //printf("APacket.pts = %d\n", packet->av_data.pts);
	  //printf("APacket.size = %d\n", packet->av_data.size);
	  //printf("APacket->data = 0x%x\n", &(packet->av_data));

      packet_queue_put(&(global_video_state->videoq), (AVPacket *)&(packet->av_data));
  }
  pthread_exit(NULL);
}

void * listen_video_packets(){
  while(1){
      av_packet *packet = read_av_packet(player_video_socket);
	// This returns av_packet but this is AVPacket?
	  printf("VPacket.pts = %d\n", (int)packet->av_data.pts);
	  printf("VPacket.size = %d\n", packet->av_data.size);
	  printf("APacket->data = 0x%x\n\n", (unsigned int)&(packet->av_data));

      packet_queue_put(&(global_video_state->videoq), (AVPacket *)&(packet->av_data));
  }
  pthread_exit(NULL);
}

void * listen_control_packets(){
  while(1){
    // read_control_packet();
  }
  pthread_exit(NULL);
}

char * nameserver_init(char * name){
    struct addrinfo hints2, * res2;
    memset(&hints2, 0, sizeof hints2);
	int nameserver_socket = 0;
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


	printf("[PLAYER] Connecting to nameserver...\n");
	if( connect(nameserver_socket, res2->ai_addr, res2->ai_addrlen) == -1 ){
			perror("connect");
			exit(1);
	}

	// Send the find request
	char headerCode = FIND;
	write(nameserver_socket, &headerCode, 1);
	int size = strlen(name);
	write(nameserver_socket, &size, sizeof(int)); 
	write(nameserver_socket, name, size);


	// Receive the response from the nameserver
	size = 0;
	read(nameserver_socket, &size, sizeof(int));
	printf("Size: %d\n", size);
	char * ip = malloc(size+1);
	memset(ip, 0, size+1);
	read(nameserver_socket, ip, size);
	return ip;
}

int socket_connect(char * hostname_in, char * port_in, int protocol_in){
  struct addrinfo hints2, * res2;
  memset(&hints2, 0, sizeof hints2);
  hints2.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
  hints2.ai_socktype = protocol_in;
  hints2.ai_flags = AI_PASSIVE | AI_NUMERICSERV;     // fill in my IP for me
  int sock;

  if((getaddrinfo(hostname_in, port_in, &hints2, &res2)) != 0){
      perror("getaddrinfo");
      exit(1);
  }

 	if( (sock = socket(res2->ai_family, res2->ai_socktype, res2->ai_protocol)) == -1){
		perror("socket");
		exit(1);
	}

  if( protocol_in == SOCK_STREAM){ 
    if( connect(sock, res2->ai_addr, res2->ai_addrlen) == -1 ){
        perror("connect");
        exit(1);
    }
  } else if ( protocol_in == SOCK_DGRAM){
    ;// #WINNING
  }
  return sock;
}

void parse_nameserver_msg(char * ip){

	// Parse the message from the nameserver
	int size = 0;
	printf("Parsing %s\n",ip);
	strstp(ip, "#", &size);
	ip+= size;
	hostname = strstp(ip, ":", &size);
	ip+= size;
	port = strstp(ip, "#", &size);
	ip+= size;

	if(ip[0] == TCP)
    protocol = SOCK_STREAM;
	else if(ip[0] == UDP)
    protocol = SOCK_DGRAM;
	
	printf("TARGET: %s:%s [%s]\n", hostname, port, ip);

  
/*
    printf("Conected client \n");
	char * buf = malloc( 500 );
	memset(buf, 0 , 500);
	
	struct timeb tp;
	int t1, t2;

	ftime(&tp);
	t1 = (tp.time * 1000) + tp.millitm;
	//printf("Start: %d\n", tp.millitm);

	int dataread = 0;
	if( ( dataread = read(player_control_socket, buf, 25)) == -1){
		perror("read");
		exit(1);
	}

	ftime(&tp);
	t2 = (tp.time * 1000) + tp.millitm;
	printf("Elapsed Time: %d\n", t2-t1);


	int * t3 = malloc(sizeof(int));
	*t3 = t2-t1;
	if( write(player_control_socket, t3, sizeof(int))== -1 ){
		perror("write");
		exit(1);
	}

	printf("Received %d bytes: %s\n", dataread, buf);
*/
}

void keyboard_send()
{
	pantilt_packet* pt = NULL;
	while (SDL_PollEvent(&event))   //Poll our SDL key event for any keystrokes.
	{
	switch(event.type) {
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
				case SDLK_LEFT:
					pt = generate_pan_packet(-250);
				break;
      		case SDLK_RIGHT:
					pt = generate_pan_packet(250);
				break;
      		case SDLK_UP:
					pt = generate_tilt_packet(-150);
				break;
      		case SDLK_DOWN:
					pt = generate_tilt_packet(150);
				break;
				default:
				break;
			}
		}
		if(pt != NULL)
		{
			HTTP_packet* http = pantilt_to_network_packet(pt);
			if( write(player_control_socket, http->message, http->length)== -1 ){
				perror("player_client.c keyboard send write error");
				exit(1);
			}
			destroy_HTTP_packet(http);
			free(pt);
			pt = NULL;
		}
	}
}

av_packet * read_av_packet(int socket)
{
	int size = 0;
	if( read( socket, &size, sizeof(int))==0){
		perror("read");
	}
	HTTP_packet* np = create_HTTP_packet(size);
    int len = xread(socket, np);
	printf("Packet Type: %d, %d=%d\n", get_packet_type(np), size,len);
	av_packet* cp = to_av_packet(np);

  // Keep track of incoming bandwidth
  pthread_mutex_lock(&bytes_received_mutex);
  bytes_received += np->length;
  pthread_mutex_unlock(&bytes_received_mutex);
  
	printf("read_av_packet size: %d\n", cp->av_data.size);
	destroy_HTTP_packet(np);
	return cp;
}

control_packet * read_control_packet()
{
	HTTP_packet* np = create_HTTP_packet( sizeof(control_packet)+1 );
  int len = 0;
  len = xread(player_control_socket, np);
  if( len <= 0 )
    perror("xread == 0 :( :( :(");
	control_packet* cp = to_control_packet(np);

  // Keep track of incoming bandwidth
  pthread_mutex_lock(&bytes_received_mutex);
  bytes_received += np->length;
  pthread_mutex_unlock(&bytes_received_mutex);

	destroy_HTTP_packet(np);
	return cp;
}
