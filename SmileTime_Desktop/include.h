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

#include <signal.h>
#include <time.h>

#include "network_packet.h"
#include "helper.h"

int stopRecording;

#define CONTROL_PORT 1336
#define CONTROL_PORT_S "1336"
#define NAMESERVER_LISTEN_PORT 1337
#define NAMESERVER_LISTEN_PORT_S "1337"
#define AUDIO_PORT 1338
#define AUDIO_PORT_S "1338"
#define VIDEO_PORT 1339
#define VIDEO_PORT_S "1339"
#define AV_PORT 1339
#define AV_PORT_S "1339"
#define BACKLOG 20

#define NAMESERVER_IP "173.230.140.232"

#define TCP '0'
#define UDP '1'

#define NAME "name"

#define FIND 0
#define ADD  1
#define EXIT 2

// UDP: SOCK_DGRAM
// TCP: SOCK_STREAM

char * getIP();

#endif
