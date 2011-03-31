#ifndef RECORDER_SERVER_H
#define RECORDER_SERVER_H

#include "include.h"
#include "video_play.h"

void init_server();
void init_control_connection();
int recorder_socket;
int acceptfd;
void establish_peer_connection();
void listen_control_packets();
void register_nameserver();
char * nameServerMsg(char * name, char * ip, char * port, char * protocol, int * size);
char *  getIP();

#endif
