#ifndef INCLUDE_H
#define INCLUDE_H

#include <sys/types.h>          /* See NOTES */
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

#define NAMESERVER_LISTEN_PORT 1337
#define LISTEN_PORT 1336
#define BACKLOG 20

#endif
