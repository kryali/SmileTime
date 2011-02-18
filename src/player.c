#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "video_play.h"
#include "audio_play.h"
#include "io_tools.h"

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
  videoStream = -1;
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


int frameFinished;
AVPacket packet;
i=0;

while(av_read_frame(pFormatCtx, &packet)>=0) {
// Is this a packet from the video stream?
if(packet.stream_index==videoStream) {
// Decode video frame
  avcodec_decode_video(pCodecCtx, pFrame, &frameFinished,
                       packet.data, packet.size);
  
  // Did we get a video frame?
  if(frameFinished) {
  // Convert the image from its native format to RGB
      img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24, 
          (AVPicture*)pFrame, pCodecCtx->pix_fmt, 
    pCodecCtx->width, pCodecCtx->height);

      // Save the frame to disk
      if(++i<=5)
        SaveFrame(pFrameRGB, pCodecCtx->width, 
                  pCodecCtx->height, i);
  }
}
  
// Free the packet that was allocated by av_read_frame
av_free_packet(&packet);
}

  video_play_init();
  audio_play_init();

  while(frames_to_play > 0)
  {


    av_read_frame(pFormatCtx, &packet)>=0



    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video(pCodecCtx, pFrame, &frameFinished,
                           packet.data, packet.size);
      
      // Did we get a video frame?
      if(frameFinished) {
        // Convert the image from its native format to RGB
        img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24, 
              (AVPicture*)pFrame, pCodecCtx->pix_fmt, 
              pCodecCtx->width, pCodecCtx->height);

        // Save the frame to disk
        if(++i<=5)
          SaveFrame(pFrameRGB, pCodecCtx->width, 
                    pCodecCtx->height, i);
      }
    
    /*audio_read();
    video_read();

    audio_segment_decompress();
    audio_segment_playback();
    video_frame_decompress();
    */

    printf("[MAIN] Synchronize your video frame play to the audio play.\n");
    usleep(1000000);
    //video_frame_display();

    frames_to_play --;
  }

  printf("Playback Frame Rate: *** fps\n");
  printf("[MAIN] Quit player\n");
  return 0;
}
