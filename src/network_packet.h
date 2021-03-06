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

#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#define CONTROL_PACKET 82
#define PANTILT_PACKET 1
#define AV_PACKET 2

#define PAN 0
#define TILT 1

typedef struct __HTTP_packet
{
	void *message;
	size_t length;
} HTTP_packet;

typedef struct __control_packet{
  AVCodecContext audio_codec_ctx;
  AVCodecContext video_codec_ctx;
  AVCodec audio_codec;
  AVCodec video_codec;
	//int elapsed_time;
} control_packet;

typedef struct __pantilt_packet{
	int type;//pan or tilt
	int distance;
} pantilt_packet;

typedef struct __av_packet{
	AVPacket av_data;
} av_packet;

HTTP_packet* create_HTTP_packet(int length);
HTTP_packet* create_HTTP_AV_packet(int length);
void destroy_HTTP_packet(HTTP_packet* packet);

//data structs -> network packets
HTTP_packet* control_to_network_packet(control_packet* packet);
HTTP_packet* pantilt_to_network_packet(pantilt_packet* packet);
HTTP_packet* av_to_network_packet(av_packet* packet);

//network packets -> data structs
char get_packet_type(HTTP_packet* network_packet);
control_packet* to_control_packet(HTTP_packet* network_packet);
pantilt_packet* to_pantilt_packet(HTTP_packet* network_packet);
av_packet* to_av_packet(HTTP_packet* network_packet);

//data struct creators
//control_packet* generate_control_packet(int elapsed_time);
pantilt_packet* generate_pantilt_packet(int type, int distance);
pantilt_packet* generate_pan_packet(int distance);
pantilt_packet* generate_tilt_packet(int distance);
//av_packet* generate_av_packet(AVPacket* avpacket);

#endif
