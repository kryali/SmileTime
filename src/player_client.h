#ifndef PLAYER_CLIENT_H
#define PLAYER_CLIENT_H

#include "include.h"

int player_control_socket;
int player_audio_socket;
int player_video_socket;

char * hostname;
char * port;
int protocol;


void establish_video_connection();
void establish_audio_connection();
void establish_control_connection();
void establish_peer_connections(int protocol);

int socket_connect(char * hostname, char * port, int protocol);
void parse_nameserver_msg(char * ip);

void keyboard_send();

control_packet * read_control_packet();
av_packet * read_av_packet(int socket);

pthread_t video_thread_id;
pthread_t audio_thread_id;
pthread_t control_thread_id;
char * nameserver_init(char*name);



#endif
