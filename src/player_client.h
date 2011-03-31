#ifndef PLAYER_CLIENT_H
#define PLAYER_CLIENT_H

#include "include.h"

void client_init(char * ip);
void keyboard_send();

char * nameserver_init(char*name);

int player_socket;

#endif
