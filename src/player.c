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

  // Find the first video stream
  int videoStream = -1;
  for( i=0; i<pFormatCtx->nb_streams; i++ )
    if( pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO ) {
      videoStream = i;
      break;
  }

  if(videoStream == -1)
    return -1; // Didn't find a video stream

  // Get a pointer to the codec context for the video stream
  pCodecCtx = pFormatCtx->streams[videoStream]->codec;

  AVCodec *pCodec;

  // Find the decoder for the video stream
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
