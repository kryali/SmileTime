#ifndef RECORDER_SERVER_H
#define RECORDER_SERVER_H

#include "include.h"

void init_server();
void init_control_connection();
int recorder_socket;
int acceptfd;

#endif
