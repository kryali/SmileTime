#include "player_client.h"
#include "video_play.h"

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

/*
	if( (res2->ai_protocol & SOCK_NONBLOCK) == 0 ){
		printf("Socket is blocking\n");
	}
*/
 	if( (player_socket = socket(res2->ai_family, res2->ai_socktype, res2->ai_protocol)) == -1){
		perror("socket");
		exit(1);
	}


	printf("Client connecting to server...\n");
	if( connect(player_socket, res2->ai_addr, res2->ai_addrlen) == -1 ){
			perror("connect");
			exit(1);
	}
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
	if( ( dataread = read(player_socket, buf, 25)) == -1){
		perror("read");
		exit(1);
	}

	ftime(&tp);
	t2 = (tp.time * 1000) + tp.millitm;
	printf("Elapsed Time: %d\n", t2-t1);


	int * t3 = malloc(sizeof(int));
	*t3 = t2-t1;
	if( write(player_socket, t3, sizeof(int))== -1 ){
		perror("write");
		exit(1);
	}

	printf("Received %d bytes: %s\n", dataread, buf);
*/
}

void keyboard_send()
{
	pantilt_packet pt;
	pt.distance = 0;
	while (SDL_PollEvent(&event))   //Poll our SDL key event for any keystrokes.
	{
	switch(event.type) {
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
				case SDLK_LEFT:
					pt.type = PAN;
					pt.distance = -250;
				break;
      		case SDLK_RIGHT:
					pt.type = PAN;
					pt.distance = 250;
				break;
      		case SDLK_UP:
					pt.type = TILT;
					pt.distance = -150;
				break;
      		case SDLK_DOWN:
					pt.type = TILT;
					pt.distance = 150;
				break;
				default:
					pt.distance = 0;
				break;
			}
		}
		printf("write: %d\n", pt.distance);
		if(pt.distance != 0)
		{
			HTTP_packet* http = pantilt_to_network_packet(&pt);
			if( write(player_socket, http->message, http->length)== -1 ){
				perror("write");
				exit(1);
			}
			destroy_HTTP_packet(http);
		}
	}
}
