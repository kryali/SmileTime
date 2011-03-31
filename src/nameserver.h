#ifndef NAMESERVER_H
#define NAMESERVER_H

#include "include.h"
#include "lib/linkedlist.h"

void init_server();
void message_listen();
void handle_connection(int fd);
void add_server(char * msg);
char * server_find(char * name);
char * strstp(char * str, char * stp, int * size);
char * nameServerMsg(char * name, char * ip, char * port, char * protocol, int * size);

int nameserver_socket;
list * iplist;


#endif
