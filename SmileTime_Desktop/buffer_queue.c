#include "buffer_queue.h"

void buffer_queue_init(BufferQueue *q) {
  memset(q, 0, sizeof(BufferQueue));
  pthread_mutex_init(&q->mutex, NULL);
}

int buffer_queue_put(BufferQueue *q, Buffer *buff) {
  BufferList *buff1;
  buff1 = malloc(sizeof(BufferList));
  if (!buff1)
    return -1;
  buff1->buff = *buff;
  buff1->next = NULL;
  
	pthread_mutex_lock( &q->mutex );
  
  if (!q->last_buff)
    q->first_buff = buff1;
  else
    q->last_buff->next = buff1;
  q->last_buff = buff1;
  q->nb_packets++;
  q->size += buff1->buff.length;
  
	pthread_mutex_unlock( &q->mutex );
  return 0;
}

int buffer_queue_peek(BufferQueue *q, Buffer *buff) {
  BufferList *buff1;
  int ret = 0;
   
  pthread_mutex_lock( &q->mutex );

    buff1 = q->first_buff;
    if (buff1) {
      *buff = buff1->buff;
      ret = 1;
    }

  pthread_mutex_unlock( &q->mutex );
  return ret;
}


int buffer_queue_get(BufferQueue *q, Buffer *buff) {
  BufferList *buff1;
  int ret = 0;
   
  pthread_mutex_lock( &q->mutex );

    buff1 = q->first_buff;
    if (buff1) {
      q->first_buff = buff1->next;
      if (!q->first_buff)
        q->last_buff = NULL;
      q->nb_packets--;
      q->size -= buff1->buff.length;
      *buff = buff1->buff;
      free(buff1);
      ret = 1;
    }

  pthread_mutex_unlock( &q->mutex );
  return ret;
}
