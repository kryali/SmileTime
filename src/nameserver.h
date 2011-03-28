#ifndef NAMESERVER_H
#define NAMESERVER_H

#include "include.h"
#include "lib/linkedlist.h"

void init_server();
void message_listen();

int nameserver_socket;
list * iplist;

#endif
