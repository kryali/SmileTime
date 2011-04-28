#include "smiletime.h"

void usage()
{
    printf("[smiletime] Usage: ./recorder USERNAME PROTOCOL(TCP:0 UDP:1) PORT\n\
    Press CTRL+C to exit\n\n");
}

void onExit()
{
    printf("[smiletime] Quitting\n");
    stopRecording = 1;
    exit(0);
}

// Thread function to encode and send video
void * startVideoEncoding(){
	while( stopRecording == 0){
		video_frame_copy();
		if(streaming == 1)
			video_frame_send();
		video_frame_mjpg_to_yuv();
		video_frame_display();
	}
	pthread_exit(NULL);
}

// Thread function to encode and send audio
void * startAudioEncoding(){
	while( stopRecording == 0){
		audio_segment_copy();
		if(streaming == 1)
			audio_segment_send();
	}
	pthread_exit(NULL);
}

void * startAudioDecoding(){

	while(stopRecording == 0){
		read_audio_packet();
	}
	pthread_exit(NULL);
}

void * startVideoDecoding(){
	while(stopRecording == 0){
		video_frame_decompress();
	}
	pthread_exit(NULL);
}

// Thread function to capture and handle keyboard input (for pan tilt)
void * captureKeyboard(){
	while( stopRecording == 0){
		keyboard_capture();
	}
	pthread_exit(NULL);
}

// Thread function to capture and handle keyboard input
void * captureTextMessages(){
	while( stopRecording == 0){
		send_text_message();
	}
	pthread_exit(NULL);
}

// Registers with the nameserver
void connect_to_nameserver(int argc, char*argv[]){
	char * name = "default";
	protocol = "1";
	peer_port = CONTROL_PORT_S;
	if( argc >= 2)
		name = argv[1];
	if( argc >= 3)
		peer_port = argv[2];
	if( argc >= 4)
		protocol = argv[3];
	printf("[smiletime] about to contact nameserver...\n\tusername: %s\n\tprotocol: %s\n\tport: %s\n", name, protocol, peer_port);
	register_nameserver(name, protocol, peer_port);
}

int main(int argc, char*argv[])
{
	if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		usage();
		return 0;
	} 

	signal(SIGINT, &onExit);
	streaming = 0;
  	stopRecording = 0;

	// * Initializiations * 
	video_record_init();
	video_play_init();
	audio_play_init();
	audio_record_init();
	init_udp_av();
	init_udp_audio();

	// * Start recording and encoding audio and video, capturing keyboard input, and prepare for AV streaming * 
	pthread_create(&video_capture_thread_id, NULL, startVideoEncoding, NULL);

	pthread_create(&audio_capture_thread_id, NULL, startAudioEncoding, NULL);
	//startAVReceiving();
	pthread_create(&control_network_thread_id, NULL, (void*)listen_control_packets,(void*) NULL);
	pthread_create(&audio_decode_thread_id, NULL, startAudioDecoding, NULL);
	pthread_create(&video_decode_thread_id, NULL, startVideoDecoding, NULL);

	pthread_create(&keyboard_thread_id, NULL,  captureKeyboard, NULL);
	pthread_create(&text_send_thread_id, NULL,  captureTextMessages, NULL);

	// * Connect to nameserver * 
	connect_to_nameserver(argc, argv);

	// * Start listening for peer connections. *
	listen_peer_connections(strToInt(peer_port));

	// * Establish control and audio/video connections for multiple users. * 
	while(stopRecording == 0){
		printf("[smiletime] waiting for a peer connection.\n");
		accept_peer_connection(recorder_control_socket, SOCK_STREAM);
		printf("[smiletime] received a new peer connection.\n");
		streaming = 1;
	}

	// * Wait for threads to exit * 
	pthread_join(video_capture_thread_id, NULL);
	pthread_join(audio_capture_thread_id, NULL);
	pthread_join(keyboard_thread_id, NULL);
	pthread_join(text_send_thread_id, NULL);
	pthread_join(control_network_thread_id, NULL);



	// * Exit *
	sdl_quit();
	video_close();
	audio_close();
	audio_play_close();
	printf("[RECORDER] Quit Successfully\n");
	return 0;
}
