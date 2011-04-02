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

#include "recorder_server.h"
#include "recorder_client.h"

#include "include.h"


char* defaultPath = "/nmnt/work1/cs414/G6/";
FILE* output;
pthread_mutex_t fileMutex;
pthread_t control_network_thread_id;
pthread_t video_network_thread_id;
pthread_t audio_network_thread_id;
pthread_t video_thread_id;
pthread_t audio_thread_id;
pthread_t keyboard_thread_id;

void usage()
{
    printf("\n\
    recorder USAGE:    ./recorder USERNAME (TCP:0 UDP:1) PORT\n\
    Press CTRL+C to exit\n\n");
}

void onExit()
{
    printf("[RECORDER] Exiting\n");
    stopRecording = 1;
}

void * startVideoEncoding(){
	int bufferIndex = 0;
	int elapsedTime = 0;
	int frames = 0;
	ftime(&startTime);

	while( stopRecording == 0){
		bufferIndex = video_frame_copy();
		video_frame_display( bufferIndex );
		video_frame_compress( bufferIndex );

		/*frames++;
		ftime(&currentTime);
		elapsedTime =  ((currentTime.time-startTime.time) * 1000 ) + ((currentTime.millitm-startTime.millitm) ); 
		framesps = (float)elapsedTime / 1000;*/
	}
	//printf("frames: %d\n", frames);
	//printf("time ms: %d\n", elapsedTime);
	pthread_exit(NULL);
}

void * startAudioEncoding(){
	while( stopRecording == 0){
		audio_segment_copy();
		audio_segment_compress();
	}
	pthread_exit(NULL);
}

void * captureKeyboard(){
	while( stopRecording == 0){
		keyboard_capture();
	}
	pthread_exit(NULL);
}

int main(int argc, char*argv[])
{
	if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		usage();
		return 0;
	} 
	stopRecording = 0;
	signal(SIGINT, &onExit);

	AVOutputFormat *fmt;
	AVFormatContext *oc;
    
	avcodec_init();
	av_register_all();
	
	fmt = av_guess_format(NULL, ".mkv", NULL);
	if (!fmt) {
		fprintf(stderr, "Could not find suitable output format\n");
		exit(1);
	}
	fmt->flags = fmt->flags | AVFMT_NOFILE;

	// allocate the output media context
	oc = avformat_alloc_context();
	if (!oc) {
		fprintf(stderr, "Memory error\n");
		exit(1);
	}
	oc->oformat = fmt;
 
	// set the output parameters (must be done even if no parameters).
	if (av_set_parameters(oc, NULL) < 0) {
		fprintf(stderr, "Invalid output format parameters\n");
		exit(1);
	}
  
	// Initializiations
	video_record_init(fmt, oc);
	video_play_init();
//	audio_record_init(fmt, oc);

	pthread_create(&video_thread_id, NULL, startVideoEncoding, NULL);
	//pthread_create(&audio_thread_id, NULL, startAudioEncoding, NULL);
	pthread_create(&keyboard_thread_id, NULL,  captureKeyboard, NULL);

	char * name = "default";
	char* protocol = "1";
	char* control_port = CONTROL_PORT_S;
	if( argc >= 2)
		name = argv[1];
	if( argc >= 3)
		protocol = argv[2];
	if( argc >= 4)
		control_port = argv[3];
	register_nameserver(name, protocol, control_port);
	if(protocol[0] == TCP)
		establish_peer_connections(SOCK_STREAM);
	else if(protocol[0] == UDP)
		establish_peer_connections(SOCK_DGRAM);
	else
		exit(1);

	//networking
	pthread_mutex_init(&fileMutex, NULL);
	pthread_create(&control_network_thread_id, NULL, listen_control_packets, NULL);
	pthread_create(&video_network_thread_id, NULL, stream_video_packets, NULL);
	pthread_create(&audio_network_thread_id, NULL, stream_audio_packets, NULL);

	pthread_join(video_thread_id, NULL);
	//pthread_join(audio_thread_id, NULL);	
	pthread_join(keyboard_thread_id, NULL);
	pthread_join(control_network_thread_id, NULL);
	pthread_join(video_network_thread_id, NULL);
	pthread_join(audio_network_thread_id, NULL);

	pthread_mutex_destroy(&fileMutex);

	av_write_trailer(oc);
	sdl_quit();
	video_close();
//	audio_close();
	
	/* free the stream */
	av_free(oc);
	printf("[MAIN] Quit recorder\n");
	return 0;
}
