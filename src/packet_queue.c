#include "packet_queue.h"

void recorder_packet_queue_init(RecorderPacketQueue *q) {
  memset(q, 0, sizeof(RecorderPacketQueue));
  pthread_mutex_init(&q->mutex, NULL);
}

int recorder_packet_queue_put(RecorderPacketQueue *q, AVPacket *pkt) {
  AVPacketList *pkt1;
  if(av_dup_packet(pkt) < 0) {
    return -1;
  }
  pkt1 = av_malloc(sizeof(AVPacketList));
  if (!pkt1)
    return -1;
  pkt1->pkt = *pkt;
  pkt1->next = NULL;
  
	pthread_mutex_lock( &q->mutex );
  
  if (!q->last_pkt)
    q->first_pkt = pkt1;
  else
    q->last_pkt->next = pkt1;
  q->last_pkt = pkt1;
  q->nb_packets++;
  q->size += pkt1->pkt.size;
  
	pthread_mutex_unlock( &q->mutex );
  return 0;
}

int recorder_packet_queue_get(RecorderPacketQueue *q, AVPacket *pkt) {
  AVPacketList *pkt1;
  int ret = 0;
   
  pthread_mutex_lock( &q->mutex );

    pkt1 = q->first_pkt;
    if (pkt1) {
      q->first_pkt = pkt1->next;
      if (!q->first_pkt)
        q->last_pkt = NULL;
      q->nb_packets--;
      q->size -= pkt1->pkt.size;
      *pkt = pkt1->pkt;
      av_free(pkt1);
      ret = 1;
    }

  pthread_mutex_unlock( &q->mutex );
  return ret;
}
