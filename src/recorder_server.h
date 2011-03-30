#ifndef RECORDER_SERVER_H
#define RECORDER_SERVER_H

#include "include.h"

void init_server();

int recorder_socket;
int acceptfd;
void listen_connections();
void register_nameserver();
char *  getIP();

#endif
