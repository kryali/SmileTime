#include "audio_play.h"

void audio_play_init()
{
    printf("[A_PLAY] This function initiates the sound device that plays audio\n");
}

void audio_segment_decompress()
{
    printf("[A_PLAY] This function decompresses the audio data\n");
}

void audio_segment_playback()
{
    printf("[A_PLAY] This function plays the sound out to devices\n");
}

void init_udp_audio(){
	struct sockaddr_in si;
	if ((audio_socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		perror("socket");

	memset((char *) &si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(AUDIO_PORT);
	si.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(audio_socket, &si, sizeof(si))==-1)
		perror("bind");
	printf("[AUDIO] UDP Socket is bound\n");
	audioBuffer = malloc(AUDIO_PACKET_SIZE);
	memset(audioBuffer, 0, AUDIO_PACKET_SIZE);
}

void * read_audio_packet(){
	int readbytes = 0;
	struct sockaddr_in si;
	unsigned int sLen = sizeof(si);
	memset(audioBuffer, 0, AUDIO_PACKET_SIZE);
	if( (readbytes = recvfrom(audio_socket, audioBuffer, AUDIO_PACKET_SIZE, 0, &si, &sLen))== -1){
		perror("recvfrom");
	}
//	printf("[AUDIO] Read %d bytes from the audio packet\n", readbytes);
	return audioBuffer;
}
