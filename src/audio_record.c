#include "audio_record.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>

int microphone_fd = -1;
char *microphone_name = "/dev/dsp";
short *buf;
int buf_size;
uint8_t *outbuf;
int outbuf_size;

AVCodec *codec;
AVCodecContext *c = NULL;

int channels = 1;
int sample_bits = 16; //bits per sample
int sample_rate = 44100; //samples per second
int sample_size; //size of 1 sample in bytes
int frame_size; //size of a frame in samples

void audio_record_init()
{
	printf("[A_REC] This function initializes the audio device for recording\n");
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

	// register all the codecs
	avcodec_init();
	avcodec_register_all();

	// find the lame mp3 encoder
	codec = avcodec_find_encoder(CODEC_ID_MP3);
	if (!codec) {
	fprintf(stderr, "codec not found\n");
	exit(1);
	}
	
	// Initialize the sample context
	c = avcodec_alloc_context();	
	c->sample_fmt = SAMPLE_FMT_S16;
	c->sample_rate = sample_rate;
	c->channels = 1;
	//c->bit_rate = 64000;

	// open the codec
	if( avcodec_open(c, codec) < 0) {
		fprintf(stderr, "could not open codec\n");
		exit(1);
  	}
	frame_size = c->frame_size;

	//buf
  	buf_size = frame_size * sample_size;
	printf("buf size: %d\n", buf_size);
	buf =  malloc(buf_size);
	//outbuf
	outbuf_size = 10000; // size?	
	outbuf = malloc(outbuf_size);
}

void audio_segment_copy()
{	
	printf("[A_REC] This function copies the audio segment from sound driver\n");	
	//int bytes;
	//if( (bytes = read( microphone_fd, buf, buf_size )) != buf_size )
	//	printf("Wrong number of bytes read: %d\n", bytes);
  	//FILE* file = fopen( "/home/engr/hughes11/Desktop/raw.wav", "wb" );
	//write( microphone_fd, buf, buf_size );//plays the audio back
	//fwrite( buf, 1, buf_size, file );
	//audio_segment_compress();
	//fclose(file);
}

void audio_segment_compress()
{
    printf("[A_REC] This function should compress the raw wav to MP3 or AAC\n");
		/* must be called before using avcodec lib */

	//encode the audio from buf to outbuf
	FILE* file = fopen( "/home/engr/hughes11/Desktop/raw.mp2", "wb" );

	int out_size, i, bytes;
	out_size = 0;

	for(i=0; i< 300; i++){
		if( (bytes = read( microphone_fd, buf, buf_size )) != buf_size )
			printf("Wrong number of bytes read: %d\n", bytes);
		out_size = avcodec_encode_audio(c, outbuf, outbuf_size, (buf));
		printf("Encoded %d bytes\n", out_size);
		if(out_size == 0)
			fwrite( outbuf, 1, outbuf_size, file );
		else
			fwrite( outbuf, 1, out_size, file );
	}

	fclose(file);
	//fwrite(outbuf, 1, out_size, f);
}

//Frees all memory and closes codecs.
void audio_exit()
{
	free(outbuf);	
	free(buf);
	avcodec_close(c);
	av_free(c);
	printf("closing interface");
}
