#include "audio_record.h"
#include "recorder_server.h"
#include "include.h"

int microphone_fd = -1;
char *microphone_name = "/dev/dsp";

short *audio_buf;
int audio_buf_size;
struct timeb time_of_copy;

Buffer audio_pkt;

int channels = 1;
int sample_bits = 16; //bits per sample
int sample_rate = 44100; //samples per second
int sample_size; //size of a sample in bytes
int audio_input_frame_size = 1000; //number of samples per frame

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
	ftime(&time_of_copy);
}

void audio_segment_queue()
{
	audio_pkt.timestamp = (time_of_copy.time * 1000) + time_of_copy.millitm;
	audio_pkt.length = audio_buf_size;
	audio_pkt.start = malloc(audio_pkt.length);
	memcpy(audio_pkt.start, audio_buf, audio_pkt.length);

	buffer_queue_put(audioq, &audio_pkt);
}

Buffer net_pkt;
void audio_segment_send()
{
	if(audioq->nb_packets > 0 && buffer_queue_get(audioq, &net_pkt) == 1)
	{
		av_packet av;
		av.buff = net_pkt;
		HTTP_packet* http = av_to_network_packet(&av);
		xwrite(avfd, http);
		destroy_HTTP_packet(http);
		free(net_pkt.start);
	}
}

//Frees all memory and closes codecs.
void audio_close()
{
	free(audioq);
	free(audio_buf);
	close(microphone_fd);
}
