#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "video_record.h"
#include "video_play.h"
#include "audio_record.h"
#include "io_tools.h"


char* defaultPath = "/nmnt/work1/cs414/G6/";
int stopRecording = 0;
char* filename, fbuf;

void usage()
{
    printf("\n\
    recorder USAGE:    recorder FILE_NAME\n\
    Capture the video and audio data and save to the FILE_NAME. Press CTRL+C to exit\n\n");
}

void onExit()
{
    printf("[MAIN] CTRL+C has been received. Add logic here before the program exits\n");
    stopRecording = 1;
    video_close();
    sdl_quit();
    audio_exit();
}

int main(int argc, char*argv[])
{
    if (argc != 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
      usage();
      return 0;
    }

    signal( SIGINT,&onExit);

    printf("[MAIN] I am going to record both video and audio data to the file: %s\n", filename);

    avcodec_init();
    av_register_all();
    buffers = NULL;

    video_record_init();
    video_play_init();
    audio_record_init();
    audio_segment_copy();
    audio_segment_compress();
    //panTilt_reset();
    //pan_relative(20);
    tilt_relative(150);
    int bufferIndex = 0;
    int i;
    i = 0;
    while(stopRecording == 0)
    {
  
        bufferIndex = video_frame_copy();
        encode_frame(argv[1], 0);
        video_frame_display(bufferIndex);
        //usleep(40000);
/*
        video_frame_compress();
        audio_segment_copy();
        audio_segment_compress();

        video_save();
        audio_save();

        printf("[MAIN] One frame has been captured, sleep for a while and continue...\n");
        usleep(1000000);
*/
    }

    free(filename);
    printf("[MAIN] Quit recorder\n");
    return 0;
}
