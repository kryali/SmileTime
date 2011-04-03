#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

typedef struct RecorderPacketQueue {
  AVPacketList *first_pkt, *last_pkt;
  int nb_packets;
  int size;
  pthread_mutex_t mutex;
} RecorderPacketQueue;

void recorder_packet_queue_init(RecorderPacketQueue *q);
int recorder_packet_queue_put(RecorderPacketQueue *q, AVPacket *pkt);
int recorder_packet_queue_get(RecorderPacketQueue *q, AVPacket *pkt);

#endif
