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

pthread_t control_network_thread_id;
pthread_t video_network_thread_id;
pthread_t audio_network_thread_id;
pthread_t video_thread_id;
pthread_t audio_thread_id;
pthread_t keyboard_thread_id;

int streaming;

void usage()
{
    printf("[RECORDER] Usage: ./recorder USERNAME PROTOCOL(TCP:0 UDP:1) PORT\n\
    Press CTRL+C to exit\n\n");
}

void onExit()
{
    printf("[RECORDER] Quitting\n");
    stopRecording = 1;
    exit(0);
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
		if(streaming == 1)
			video_frame_queue();
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
		if(streaming == 1)
			audio_segment_queue();
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

	signal(SIGINT, &onExit);
	streaming = 0;
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
  
	// * Initializiations * 
	video_record_init(fmt, oc);
	video_play_init();
	audio_record_init(fmt, oc);

	// * Start recording and encoding audio and video * 
	stopRecording = 0;
	pthread_create(&video_thread_id, NULL, startVideoEncoding, NULL);
	pthread_create(&audio_thread_id, NULL, startAudioEncoding, NULL);
	pthread_create(&keyboard_thread_id, NULL,  captureKeyboard, NULL);

	// * Connect to nameserver * 
	char * name = "default";
	char* protocol = "0";
	char* control_port = CONTROL_PORT_S;
	if( argc >= 2)
		name = argv[1];
	if( argc >= 3)
		protocol = argv[2];
	if( argc >= 4)
		control_port = argv[3];
	register_nameserver(name, protocol, control_port);

	// * Establish control, audio, and video connections * 
	if(protocol[0] == TCP)
		establish_peer_connections(SOCK_STREAM);
	else if(protocol[0] == UDP)
		establish_peer_connections(SOCK_DGRAM);
	else
		exit(1);

  send_init_control_packet( oc->streams[0], oc->streams[1] );

	// * Transmit data through the network *
	pthread_create(&control_network_thread_id, NULL, (void*)listen_control_packets,(void*) NULL);
	pthread_create(&video_network_thread_id, NULL, (void*)stream_video_packets, (void*)NULL);
	pthread_create(&audio_network_thread_id, NULL, (void*)stream_audio_packets, (void*)NULL);
	streaming = 1;

	// * Wait for threads to exit * 
	pthread_join(video_thread_id, NULL);
	pthread_join(audio_thread_id, NULL);	
	pthread_join(keyboard_thread_id, NULL);
	pthread_join(control_network_thread_id, NULL);
	pthread_join(video_network_thread_id, NULL);
	pthread_join(audio_network_thread_id, NULL);

	// * Exit *
//	pthread_mutex_destroy(&fileMutex);
	sdl_quit();
	video_close();
	audio_close();
	av_free(oc);
	printf("[RECORDER] Quit Successfully\n");
	return 0;
}
