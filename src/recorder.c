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
	double audio_pts, video_pts;
    
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

	dump_format(oc, 0, filename, 1);
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
	
	av_write_header(oc);
	while(stopRecording == 0)
	{
		pan_relative(50);
		video_frame_copy();
		video_frame_compress();  
		video_frame_display();
		audio_segment_copy();
		audio_segment_compress();
		
		/*if (audio_st)
			audio_pts = (double)audio_st->pts.val * audio_st->time_base.num / audio_st->time_base.den;
		else
			audio_pts = 0.0;
	
		if (video_st)
			video_pts = (double)video_st->pts.val * video_st->time_base.num / video_st->time_base.den;
		else
			video_pts = 0.0;

		//write interleaved audio and video frames
		if (!video_st || (video_st && audio_st && audio_pts < video_pts)) {
			audio_segment_write();
		} else {
			video_frame_write();
		}*/
		audio_segment_write();
		video_frame_write();

	}
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
