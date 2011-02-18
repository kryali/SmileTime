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

#include "video_record.h"
#include "video_play.h"
#include "audio_record.h"
#include "audio_play.h"
#include "io_tools.h"


char* defaultPath = "/nmnt/work1/cs414/G6/";
int stopRecording = 0;
pthread_t threads;
FILE* output;

AVOutputFormat *fmt;
AVFormatContext *oc;
AVStream *audio_st, *video_st;
double audio_pts, video_pts;
int i;

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
    free(output);
}

int main(int argc, char*argv[])
{
    //pthread_t video_encoding_thread;
    if (argc != 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
      usage(argv[1]);
      return 0;
    } 
    signal( SIGINT,&onExit);
	avcodec_init();
	av_register_all();
	output = fopen(argv[1], "wb");
	if (!output) {
		fprintf(stderr, "could not open %s\n", argv[1]);
		exit(1);
	}

	//not sure if this will work, we want mkv with h264 video and mp3 audio.
	fmt = av_guess_format(NULL, argv[1], NULL);
    	if (!fmt) {
    	    printf("Could not deduce output format from file extension: using mkv.\n");
    	    fmt = av_guess_format("mkv", NULL, NULL);
    	}
    	if (!fmt) {
    	    fprintf(stderr, "Could not find suitable output format\n");
    	    exit(1);
    	}
	fmt->video_codec = CODEC_ID_H264;
	fmt->audio_codec = CODEC_ID_MP3;

   	/* allocate the output media context */
    	oc = avformat_alloc_context();
    	if (!oc) {
    	    fprintf(stderr, "Memory error\n");
    	    exit(1);
    	}
    	oc->oformat = fmt;
    	snprintf(oc->filename, sizeof(oc->filename), "%s", argv[1]);

	/* add the audio and video streams using the default format codecs
	and initialize the codecs */
    	video_st = NULL;
    	audio_st = NULL;
    	if (fmt->video_codec != CODEC_ID_NONE) {
    	    //video_st = add_video_stream(oc, fmt->video_codec);
    	}
    	if (fmt->audio_codec != CODEC_ID_NONE) {
    	    //audio_st = add_audio_stream(oc, fmt->audio_codec);
    	}

    	/* set the output parameters (must be done even if no parameters). */
    	if (av_set_parameters(oc, NULL) < 0) {
      		fprintf(stderr, "Invalid output format parameters\n");
     		exit(1);
    	}

   
    printf("[MAIN] I am going to record both video and audio data to the file: %s\n", argv[1]);
    buffers = NULL;

    		printf("Checkpoitn A\n");
    video_record_init();
    
    		printf("Checkpoitn B\n");
    		video_play_init();
    		printf("Checkpoitn C\n");
    audio_record_init();
    		printf("Checkpoitn D\n");
    //audio_segment_copy();
    //audio_segment_compress();

    int bufferIndex = 0;
    int i;
    i = 0;
    while(stopRecording == 0)
    {
    		printf("Checkpoitn 1\n");
        video_frame_copy();
    		printf("Checkpoitn 2\n");
        //pthread_create( &video_encoding_thread, NULL, video_frame_compress, (void *)0 );
        video_frame_compress();
        
    		printf("Checkpoitn 3\n");
        video_frame_display();

    		printf("Checkpoitn 4\n");
    		audio_segment_copy();
        
    		printf("Checkpoitn 5\n");
        audio_segment_compress();

        video_save();
        audio_save();

        printf("[MAIN] One frame has been captured, sleep for a while and continue...\n");
        //usleep(1000000);

    }

    printf("[MAIN] Quit recorder\n");
    return 0;
}
