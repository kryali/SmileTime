#ifndef INCLUDE_H
#define INCLUDE_H

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

#include "network_packet.h"

#define NAMESERVER_LISTEN_PORT 1337
#define NAMESERVER_LISTEN_PORT_S "1337"
#define LISTEN_PORT 1336
#define LISTEN_PORT_S "1336"
#define BACKLOG 20

#define NAMESERVER_IP "173.230.140.232"

#define TCP "0"
#define UDP "1"

#define NAME "name"

#define FIND 0
#define ADD  1
#define EXIT 2

// UDP: SOCK_DGRAM
// TCP: SOCK_STREAM

int xwrite(int fd, void * buf, int len);
int xread(int fd, void * buf, int len);
char * getIP();

#endif
