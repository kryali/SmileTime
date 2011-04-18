#include "audio_record.h"
#include "recorder_server.h"
#include "include.h"

int microphone_fd = -1;
char *microphone_name = "/dev/dsp";

short *audio_buf;
int audio_buf_size;

Buffer audio_pkt;

int channels = 1;
int sample_bits = 16; //bits per sample
int sample_rate = 44100; //samples per second
int sample_size; //size of a sample in bytes
int audio_input_frame_size = 100; //number of samples per frame

void audio_record_init()
{
	// Open the microphone device
	microphone_fd = open( microphone_name, O_RDWR );
	if(microphone_fd == -1){
		perror("Opening microphone");
		exit(1);
	}
	// Set sampling parameters
	if( ioctl( microphone_fd, SOUND_PCM_WRITE_BITS, &sample_bits ) == -1){
		perror("Write bits failed");
		exit(1);
	}	
	if( ioctl( microphone_fd, SOUND_PCM_WRITE_RATE, &sample_rate ) == -1){
		perror("rate failed");
		exit(1);
	}
	if( ioctl( microphone_fd, SOUND_PCM_WRITE_CHANNELS, &channels ) == -1){
		perror("channels failed");
		exit(1);
	}

	sample_size = channels * sample_bits/8;

  	audio_buf_size = audio_input_frame_size * sample_size;
	audio_buf =  malloc(audio_buf_size);

	audioq = malloc(sizeof(BufferQueue));
	buffer_queue_init(audioq);
}

void audio_segment_copy()
{	
	if( (read( microphone_fd, audio_buf, audio_buf_size )) != audio_buf_size )
		perror("audio_segment_copy read: ");
}

void audio_segment_compress()
{
	audio_pkt.start = &audio_buf;
	audio_pkt.length = audio_buf_size;
}

void audio_segment_queue()
{
	buffer_queue_put(audioq, &audio_pkt);
}

Buffer net_pkt;
void audio_segment_write()
{
	//if(audioq->size > 0)
		//printf("audioqsize: %d\n", audioq->size);

	if(buffer_queue_get(audioq, &net_pkt) == 1)
	{
	  // Transmit the audio packet
		av_packet av;
		av.buff = net_pkt;
		HTTP_packet* http = av_to_network_packet(&av);
		xwrite(audiofd, http);
		destroy_HTTP_packet(http);
	}//yo dawg, do we have to free the avpacket's data here?
}

//Frees all memory and closes codecs.
void audio_close()
{
	free(audioq);
	free(audio_buf);
}
