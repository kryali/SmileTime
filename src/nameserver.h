#ifndef NAMESERVER_H
#define NAMESERVER_H

#include "include.h"
#include "lib/linkedlist.h"

#define FIND 0
#define ADD  1
#define EXIT 2

void init_server();
void message_listen();
void handle_connection(int fd);

int nameserver_socket;
list * iplist;


#endif
