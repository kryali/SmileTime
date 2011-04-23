#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

typedef struct Buffer Buffer;
typedef struct BufferList BufferList;
typedef struct BufferQueue BufferQueue;

struct Buffer{
    void * start;
    int length;
	int timestamp;
};

struct BufferList{
	Buffer buff;
	BufferList *next;
};

struct BufferQueue{
  BufferList *first_buff, *last_buff;
  int nb_packets;
  int size;
  pthread_mutex_t mutex;
};

void buffer_queue_init(BufferQueue *q);
int buffer_queue_put(BufferQueue *q, Buffer *buff);
int buffer_queue_peek(BufferQueue *q, Buffer *buff);
int buffer_queue_get(BufferQueue *q, Buffer *buff);

#endif
