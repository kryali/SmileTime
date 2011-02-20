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
FILE* output;
pthread_mutex_t fileMutex;
pthread_t video_thread_id;
pthread_t audio_thread_id;

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
}

void * startVideoEncoding(){
	int bufferIndex = 0;
	int elapsedTime = 0;
  int frames = 0;
	ftime(&startTime);

	while( stopRecording == 0){
		keyboard_capture();
		bufferIndex = video_frame_copy();
		video_frame_compress( bufferIndex );  
		video_frame_display( bufferIndex );

		pthread_mutex_lock( &fileMutex );
		video_frame_write();
		pthread_mutex_unlock( &fileMutex );
		frames++;
		ftime(&currentTime);
		elapsedTime =  ((currentTime.time-startTime.time) * 1000 ) + ((currentTime.millitm-startTime.millitm) ); 
		framesps = (float)elapsedTime / 1000;
	}
	pthread_exit(NULL);
}

void * startAudioEncoding(){
	while( stopRecording == 0){
		audio_segment_copy();
		audio_segment_compress();

		pthread_mutex_lock( &fileMutex );
		audio_segment_write();
		pthread_mutex_unlock( &fileMutex );
	}
	pthread_exit(NULL);
}

int main(int argc, char*argv[])
{
	if (argc != 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		usage();
		return 0;
	} 

	signal(SIGINT, &onExit);
	const char *filename = argv[1];
	
	AVOutputFormat *fmt;
	AVFormatContext *oc;
    
	avcodec_init();
	av_register_all();
	
	fmt = av_guess_format(NULL, filename, NULL);
	if (!fmt) {
		printf("Could not deduce output format from file extension: using mkv.\n");
		fmt = av_guess_format("mkv", NULL, NULL);
	}
	if (!fmt) {
		fprintf(stderr, "Could not find suitable output format\n");
		exit(1);
	}
	printf("v codec chosen: %d\n", fmt->video_codec);
	printf("h264:           %d\n", CODEC_ID_H264);
	printf("a codec chosen: %d\n", fmt->audio_codec);
	printf("mp3:            %d\n", CODEC_ID_MP3);
	//fmt->video_codec = CODEC_ID_H264;
	//fmt->audio_codec = CODEC_ID_MP3;

	// allocate the output media context
	oc = avformat_alloc_context();
	if (!oc) {
		fprintf(stderr, "Memory error\n");
		exit(1);
	}
	oc->oformat = fmt;
	snprintf(oc->filename, sizeof(oc->filename), "%s", filename);
 
	// set the output parameters (must be done even if no parameters).
	if (av_set_parameters(oc, NULL) < 0) {
		fprintf(stderr, "Invalid output format parameters\n");
		exit(1);
	}
  
	// open the output file
	if (!(fmt->flags & AVFMT_NOFILE)) {
		if (url_fopen(&oc->pb, filename, URL_WRONLY) < 0) {
			fprintf(stderr, "Could not open '%s'\n", filename);
			exit(1);
		}
	}
	
	video_record_init(fmt, oc);
	video_play_init();
	audio_record_init(fmt, oc);
	dump_format(oc, 0, filename, 1);


	av_write_header(oc);


	pthread_mutex_init(&fileMutex, NULL);

	pthread_create(&video_thread_id, NULL, startVideoEncoding, NULL);
	pthread_create(&audio_thread_id, NULL, startAudioEncoding, NULL);


	pthread_join(video_thread_id, NULL);	
	pthread_join(audio_thread_id, NULL);	
	pthread_mutex_destroy(&fileMutex);

	int end = time(NULL);
	//printf("fps: %d\n", frames/(end-start));
	//int fps = frames / ((end-start)/1000);
	//printf("%d\n", fps);
	av_write_trailer(oc);
	sdl_quit();
	video_close();
	audio_close();

	if (!(fmt->flags & AVFMT_NOFILE)) {
		/* close the output file */
		url_fclose(oc->pb);
		}

		/* free the stream */
		av_free(oc);
		printf("[MAIN] Quit recorder\n");
		return 0;
}
