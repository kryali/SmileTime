#include "structs.h"
int packet_queue_put(PacketQueue *q, AVPacket *pkt);
/*void onExit()
void packet_queue_init(PacketQueue *q);
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);
int our_get_buffer(struct AVCodecContext *c, AVFrame *pic);
void our_release_buffer(struct AVCodecContext *c, AVFrame *pic);
double synchronize_video(VideoState *is, AVFrame *src_frame, double pts);
int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size, double *pts_ptr);
void audio_callback(void *userdata, Uint8 *stream, int len);
int decode_interrupt_cb(void);
void usage();
int queue_picture(VideoState *is, AVFrame *pFrame, double pts);
int video_thread(void *arg);
int stream_component_open(VideoState *is, AVCodecContext* codecCtx);
int decode_thread( void *thread_arg );
void alloc_picture(void *userdata);
Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque);
void schedule_refresh(VideoState *is, int delay);
void video_display(VideoState *is);
double get_audio_clock(VideoState *is);
void video_refresh_timer(void *userdata);
void * captureKeyboard();
void* calculate_player_stats();
*/

int bytes_received;
pthread_mutex_t bytes_received_mutex;
