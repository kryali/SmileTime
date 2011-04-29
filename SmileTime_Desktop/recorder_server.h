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
int nfds;
fd_set fds;
int * peer_fd;
struct sockaddr_in * peer_info;
int seconds_elapsed;

int bytes_sent;
int bytes_received;
pthread_mutex_t bytes_sent_mutex;
pthread_mutex_t bytes_received_mutex;

void listen_peer_connections( int port );
int listen_on_port(int port, int protocol);
void accept_peer_connection(int socket, int protocol);
void accept_connection(int socket, int peerIndex, int protocol);
void send_text_message();
void init_udp_av();
void establish_video_connection();
void establish_audio_connection();
void establish_control_connection();

void listen_control_packets();
void register_nameserver(char * name, char * protocol, char * control_port);
char * nameServerMsg(char * name, char * ip, char * port, char * protocol, int * size);
char * getIP();
void* calculate_stats();
void* sendLatencyPackets();
void calculate_latency( latency_packet *l );

#endif
