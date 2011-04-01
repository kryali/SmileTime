#ifndef RECORDER_SERVER_H
#define RECORDER_SERVER_H

#include "include.h"
#include "video_play.h"

int recorder_control_socket;
int recorder_av_socket;
int acceptfd;

void init_server();
void init_av_connection();
void init_control_connection();
void establish_peer_connection();
void listen_control_packets();
void register_nameserver(char * name, char * protocol, char * control_port);
char * nameServerMsg(char * name, char * ip, char * port, char * protocol, int * size);
char *  getIP();

#endif
