#ifndef PLAYER_CLIENT_H
#define PLAYER_CLIENT_H

#include "include.h"

int player_control_socket;
int player_audio_socket;
int player_video_socket;


void establish_video_connection();
void establish_audio_connection();
void establish_control_connection();
void establish_peer_connections(int protocol);


void client_init(char * ip);
void keyboard_send();

control_packet * read_control_packet();

char * nameserver_init(char*name);



#endif
