#ifndef NETWORK_PACKET_H
#define NETWORK_PACKET_H

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/timeb.h>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "buffer_queue.h"

#define CONTROL_PACKET 0
#define PANTILT_PACKET 1
#define AUDIO_PACKET 2
#define VIDEO_PACKET 3
#define TEXT_PACKET 4
#define LATENCY_PACKET 5

#define TEXT_MAX_SIZE 140

typedef struct __HTTP_packet
{
	void *message;
	size_t length;
} HTTP_packet;

typedef struct __control_packet{
	int packetType;
	//int elapsed_time;
} control_packet;

typedef struct __pantilt_packet{
	int packetType;
	int pan;
	int tilt;
} pantilt_packet;

typedef struct __av_packet{
	int packetType;
	int length;
	int latency;
} av_packet;

typedef struct __text_packet{
	int packetType;
	char message[TEXT_MAX_SIZE];
} text_packet;

typedef struct __latency_packet{
	int packetType;
	int peer_sender; // 0 = Desktop-to-Mobile, 1 = Mobile-to-Desktop, 2 = Second Mobile-to-Desktop peer, etc.
  //long time_sent;
  int time_sent;
} latency_packet;

HTTP_packet* create_HTTP_packet(int length);
void destroy_HTTP_packet(HTTP_packet* packet);

//data structs -> network packets
HTTP_packet* control_to_network_packet(control_packet* packet);
HTTP_packet* pantilt_to_network_packet(pantilt_packet* packet);
HTTP_packet* av_to_network_packet(av_packet* packet, void* data);

//network packets -> data structs
int get_packet_type(HTTP_packet* network_packet);
control_packet* to_control_packet(HTTP_packet* network_packet);
pantilt_packet* to_pantilt_packet(HTTP_packet* network_packet);
text_packet* to_text_packet(HTTP_packet* network_packet);
latency_packet* to_latency_packet(HTTP_packet* network_packet);
av_packet* to_av_packet(HTTP_packet* network_packet);

//data struct creators
pantilt_packet* generate_pantilt_packet(int type, int pan, int tilt);

#endif
