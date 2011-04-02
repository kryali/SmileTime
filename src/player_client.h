#ifndef PLAYER_CLIENT_H
#define PLAYER_CLIENT_H

#include "include.h"

control_packet* client_init(char * ip);
void keyboard_send();

char * nameserver_init(char*name);

int player_control_socket;
int player_audio_socket;
int player_video_socket;

#endif
