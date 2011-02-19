#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "video_play.h"
#include "audio_play.h"
#include "io_tools.h"
#include "SDL/SDL.h"

#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

typedef struct PacketQueue {
  AVPacketList *first_pkt, *last_pkt;
  int nb_packets;
  int size;
  SDL_mutex *mutex;
  SDL_cond *cond;
} PacketQueue;

PacketQueue audioq;


void packet_queue_init(PacketQueue *q) {
  memset(q, 0, sizeof(PacketQueue));
  q->mutex = SDL_CreateMutex();
  q->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
  AVPacketList *pkt1;
  if(av_dup_packet(pkt) < 0) {
    return -1;
  }
  pkt1 = av_malloc(sizeof(AVPacketList));
  if (!pkt1)
    return -1;
  pkt1->pkt = *pkt;
  pkt1->next = NULL;
  
  
  SDL_LockMutex(q->mutex);
  
  if (!q->last_pkt)
    q->first_pkt = pkt1;
  else
    q->last_pkt->next = pkt1;
  q->last_pkt = pkt1;
  q->nb_packets++;
  q->size += pkt1->pkt.size;
  SDL_CondSignal(q->cond);
  
  SDL_UnlockMutex(q->mutex);
  return 0;
}

int quit = 0;

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
  AVPacketList *pkt1;
  int ret;
  
  SDL_LockMutex(q->mutex);
  
  for(;;) {
    
    if(quit) {
      ret = -1;
      break;
    }

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
      break;
    } else if (!block) {
      ret = 0;
      break;
    } else {
      SDL_CondWait(q->cond, q->mutex);
    }
  }
  SDL_UnlockMutex(q->mutex);
  return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

  static AVPacket pkt;
  static uint8_t *audio_pkt_data = NULL;
  static int audio_pkt_size = 0;

  int len1, data_size;

  for(;;) {
    while(audio_pkt_size > 0) {
      data_size = buf_size;
      len1 = avcodec_decode_audio2(aCodecCtx, (int16_t *)audio_buf, &data_size, 
				  audio_pkt_data, audio_pkt_size);
      if(len1 < 0) {
        /* if error, skip frame */
        audio_pkt_size = 0;
        break;
      }
      audio_pkt_data += len1;
      audio_pkt_size -= len1;
      if(data_size <= 0) {
        /* No data yet, get more frames */
        continue;
      }
      /* We have data, return it and come back for more later */
      return data_size;
    }
    if(pkt.data)
      av_free_packet(&pkt);

    if(quit) {
      return -1;
    }

    if(packet_queue_get(&audioq, &pkt, 1) < 0) {
      return -1;
    }
    audio_pkt_data = pkt.data;
    audio_pkt_size = pkt.size;
  }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {

  AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
  int len1, audio_size;

  static uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
  static unsigned int audio_buf_size = 0;
  static unsigned int audio_buf_index = 0;

  while(len > 0) {
    if(audio_buf_index >= audio_buf_size) {
      /* We have already sent all our data; get more */
      audio_size = audio_decode_frame(aCodecCtx, audio_buf,
                                      sizeof(audio_buf));
      if(audio_size < 0) {
        /* If error, output silence */
        audio_buf_size = 1024;
        memset(audio_buf, 0, audio_buf_size);
      } else {
        audio_buf_size = audio_size;
      }
      audio_buf_index = 0;
    }
    len1 = audio_buf_size - audio_buf_index;
    if(len1 > len)
      len1 = len;
    memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
    len -= len1;
    stream += len1;
    audio_buf_index += len1;
  }
}


int decode_interrupt_cb(void) {
  return quit;
}



int frames_to_play = 5;

void usage()
{
    printf("\n\
    recorder USAGE:    player FILE_NAME\n\
    Play the video and audio data saved in the FILE_NAME. Press CTRL+C to exit\n\n");
}

void onExit()
{
    printf("[MAIN] CTRL+C has been received. Add logic here before the program exits\n");

    frames_to_play = 0;
}

int main(int argc, char*argv[])
{
  if (argc != 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
  {
    usage();
    return 0;
  }

  signal( SIGINT,&onExit);

  printf("[MAIN] I am going to play both video and audio data from file: %s\n", argv[1]);

  av_register_all();

  AVFormatContext *pFormatCtx;

  // Open video file
  if(av_open_input_file(&pFormatCtx, argv[1], NULL, 0, NULL)!=0)
    return -1; // Couldn't open file

  // Retrieve stream information
  if(av_find_stream_info(pFormatCtx)<0)
    return -1; // Couldn't find stream information

  // Dump information about file onto standard error
  dump_format(pFormatCtx, 0, argv[1], 0);

  int i;
  AVCodecContext *pCodecCtx;
  AVCodecContext *aCodecCtx;

  // Find the first video stream
  int videoStream = -1;
  int audioStream = -1;
  for( i=0; i<pFormatCtx->nb_streams; i++ )
  {
    if( pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO ) {
      videoStream = i;
    }
    if( pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO && audioStream < 0 ) {
      audioStream = i;
    }
  }
  if(videoStream == -1)
    return -1; // Didn't find a video stream
  if(audioStream == -1)
    printf( "No audio stream" );
    //return -1; // Didn't find a audio stream

  // Get a pointer to the codec context for the video stream
  pCodecCtx = pFormatCtx->streams[videoStream]->codec;
  aCodecCtx = pFormatCtx->streams[audioStream]->codec;

  SDL_AudioSpec wanted_spec;
  SDL_AudioSpec spec;
  wanted_spec.freq = aCodecCtx->sample_rate;
  wanted_spec.format = AUDIO_S16SYS;
  wanted_spec.channels = aCodecCtx->channels;
  wanted_spec.silence = 0;
  wanted_spec.samples = 1024; //SDL audio buffer size
  wanted_spec.callback = audio_callback;
  wanted_spec.userdata = aCodecCtx;

  if(SDL_OpenAudio(&wanted_spec, &spec) < 0) {
    fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
    return -1;
  }

  // Find the decoder for the audio stream
  AVCodec         *aCodec;
  aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
  if(!aCodec) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1;
  }
  avcodec_open(aCodecCtx, aCodec);

  packet_queue_init(&audioq);
  SDL_PauseAudio(0);

  // Find the decoder for the video stream
  AVCodec *pCodec;
  pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec == NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  // Open codec
  if( avcodec_open(pCodecCtx, pCodec) < 0 )
    return -1; // Could not open codec

  AVFrame *pFrame;

  // Allocate video frame
  pFrame = avcodec_alloc_frame();

  //video_play_init();
  //audio_play_init();

  SDL_Surface *screen;

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    exit(1);
  }


  screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
  if(!screen) {
    fprintf(stderr, "SDL: could not set video mode - exiting\n");
    exit(1);
  }

  SDL_Overlay     *bmp;
  bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height,
                             SDL_YV12_OVERLAY, screen);
  int frameFinished;
  AVPacket packet;
  static struct SwsContext *img_convert_ctx;
  

  //while(frames_to_play > 0)
  while( av_read_frame(pFormatCtx, &packet) >= 0 )
  {
    // Is this a packet from the video stream?
    if( packet.stream_index == videoStream ) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
                           &packet);
      SDL_Rect rect;

      if(frameFinished) {
        SDL_LockYUVOverlay(bmp);

        AVPicture pict;
        pict.data[0] = bmp->pixels[0];
        pict.data[1] = bmp->pixels[2];
        pict.data[2] = bmp->pixels[1];

        pict.linesize[0] = bmp->pitches[0];
        pict.linesize[1] = bmp->pitches[2];
        pict.linesize[2] = bmp->pitches[1];

        // Convert the image into YUV format that SDL uses
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, 
                        PIX_FMT_YUV420P, 
                        pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, 
                        NULL, NULL, NULL);

        sws_scale(img_convert_ctx, pFrame->data, 
               pFrame->linesize, 0, 
               pCodecCtx->height, 
               pict.data, pict.linesize);
         
        // Display the video
        SDL_UnlockYUVOverlay(bmp);
        rect.x = 0;
        rect.y = 0;
        rect.w = pCodecCtx->width;
        rect.h = pCodecCtx->height;
        SDL_DisplayYUVOverlay(bmp, &rect);
      }   
    } else if(packet.stream_index == audioStream) {
      packet_queue_put(&audioq, &packet);
    } else {
      av_free_packet(&packet);
    }


    /*audio_read();
    video_read();

    audio_segment_decompress();
    audio_segment_playback();
    video_frame_decompress();
    */

    printf("[MAIN] Synchronize your video frame play to the audio play.\n");
    //usleep(1000000);
    //video_frame_display();

    //frames_to_play --;
  }

  // Free the YUV frame
  av_free(pFrame);

  // Close the codec
  avcodec_close(pCodecCtx);

  // Close the video file
  av_close_input_file(pFormatCtx);


  printf("Playback Frame Rate: *** fps\n");
  printf("[MAIN] Quit player\n");
  return 0;
}
