#ifndef HELPER_H
#define HELPER_H

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

//char* strstp(char * str, char * stp, int * size);
int xwrite(int fd, HTTP_packet* np);
int xread(int fd, HTTP_packet* np);

#endif
