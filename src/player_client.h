#ifndef PLAYER_CLIENT_H
#define PLAYER_CLIENT_H

#include "include.h"
#include "structs.h"


int player_control_socket;
int player_audio_socket;
int player_video_socket;
VideoState * global_video_state;

char * hostname;
char * port;
int protocol;

void listen_packets();

void * listen_audio_packets();
void * listen_video_packets();
void * listen_control_packets();

void establish_video_connection();
void establish_audio_connection();
void establish_control_connection();
void establish_peer_connections();

int socket_connect(char * hostname, char * port, int protocol);
void init_gis(VideoState * global_video_state_in);
void parse_nameserver_msg(char * ip);

void keyboard_send();

control_packet * read_control_packet();
av_packet * read_av_packet(int socket);

pthread_t video_thread_id;
pthread_t audio_thread_id;
pthread_t control_thread_id;
char * nameserver_init(char*name);


static int rtt;
void client_calculate_rtt();





#endif
