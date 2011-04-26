#ifndef RECORDER_SERVER_H
#define RECORDER_SERVER_H

#include "include.h"
#include "video_play.h"
#include "video_record.h"
#include "audio_record.h"

#define MAX_PEERS 4
#define SOCKETS_PER_PEER 2

int recorder_control_socket;

int numPeers;
int * peer_fd;
struct sockaddr_storage * peer_info;

void listen_peer_connections( int port );
int listen_on_port(int port, int protocol);
void accept_peer_connection(int socket, int protocol);
void accept_connection(int socket, int peerIndex, int protocol);
void establish_video_connection();
void establish_audio_connection();
void establish_control_connection();

//void listen_control_packets();
void register_nameserver(char * name, char * protocol, char * control_port);
char * nameServerMsg(char * name, char * ip, char * port, char * protocol, int * size);
char * getIP();
void start_stats_timer();

#endif
