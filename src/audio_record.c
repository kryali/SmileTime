#include "audio_record.h"

int microphone_fd = -1;
char *microphone_name = "/dev/dsp";
short *buf;
int buf_size;
uint8_t *outbuf;
int outbuf_size;

int channels = 1;
int sample_bits = 16; //bits per sample
int sample_rate = 44100; //samples per second
int sample_size; //size of a sample in bytes
int frame_size; //number of samples per frame

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
	// register all the codecs
	//avcodec_init();
	//avcodec_register_all();

	// find the lame mp3 encoder
	audio_codec = avcodec_find_encoder(CODEC_ID_MP3);
	if (!audio_codec) {
	fprintf(stderr, "audio_codec not found\n");
	exit(1);
	}
	// Initialize the sample context
	audio_context = avcodec_alloc_context();	
	audio_context->sample_fmt = SAMPLE_FMT_S16;
	audio_context->sample_rate = sample_rate;
	audio_context->channels = 1;
	//audio_context->bit_rate = 64000;

	// open the codec
	if( avcodec_open(audio_context, audio_codec) < 0) {
		fprintf(stderr, "could not open audio_codec\n");
		exit(1);
  	}
	frame_size = audio_context->frame_size;
	sample_size = channels * sample_bits/8;

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
	int bytes;
	if( (bytes = read( microphone_fd, buf, buf_size )) != buf_size )
		printf("Wrong number of bytes read: %d\n", bytes);
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
	//FILE* file = fopen( "/home/engr/hughes11/Desktop/raw.mp2", "wb" );

	int out_size = 0;
	out_size = avcodec_encode_audio(audio_context, outbuf, outbuf_size, (buf));
	//printf("Encoded %d bytes\n", out_size);
	//if(out_size == 0)
	//	fwrite( outbuf, 1, outbuf_size, file );
	//else
	//	fwrite( outbuf, 1, out_size, file );
	//fclose(file);
	//fwrite(outbuf, 1, out_size, f);
}

//Frees all memory and closes codecs.
void audio_exit()
{
	free(outbuf);	
	free(buf);
	avcodec_close(audio_context);
	av_free(audio_context);
	printf("closing interface");
}
