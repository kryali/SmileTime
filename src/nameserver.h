#ifndef NAMESERVER_H
#define NAMESERVER_H

#include "lib/linkedlist.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#define NAMESERVER_LISTEN_PORT 1337
#define NAMESERVER_LISTEN_PORT_S "1337"
#define LISTEN_PORT 1336
#define LISTEN_PORT_S "1336"
#define BACKLOG 20

#define NAMESERVER_IP "127.0.0.1"

#define TCP '0'
#define UDP '1'

#define NAME "name"

#define FIND 0
#define ADD  1
#define EXIT 2


void init_server();
void message_listen();
void handle_connection(int fd);
void add_server(char * msg);
char * server_find(char * name);
char * nameServerMsg(char * name, char * ip, char * port, char protocol, int * size);
//char* strstp(char * str, char * stp, int * size);

int nameserver_socket;
list * iplist;


#endif
