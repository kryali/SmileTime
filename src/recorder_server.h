#ifndef RECORDER_SERVER_H
#define RECORDER_SERVER_H

#include "include.h"

void init_server();
void init_control_connection();
int recorder_socket;
int acceptfd;
void listen_connections();
void register_nameserver();
char * nameServerMsg(char * name, char * ip, char * port, int * size);
char *  getIP();

#endif
