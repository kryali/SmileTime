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

#include "recorder_server.h"
#include "network_packet.h"

char* strstp(char * str, char * stp, int * size);
int xwrite(int peer_fd_offset, HTTP_packet* np);
int xread(int peer_fd_offset, HTTP_packet* np);
int strToInt(char* str);
int mjpeg2Jpeg(char ** jpg, const char *buffer, const int size);

#endif
