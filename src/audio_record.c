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

#define AUDIO_FORMAT SND_PCM_FORMAT_S16_LE

// http://www.equalarea.com/paul/alsa-audio.html
int microphone_fd = -1;
char* microphone_name = "default";
short * buf;

// Microphone global vars
snd_pcm_t *capture_handle;
FILE *f;
uint8_t *outbuf;
AVCodec *codec;
AVCodecContext *c = NULL;

int frame_size = 16;
int sample_rate = 8000;
int length = 5;

void audio_record_init()
{
	printf("[A_REC] This function initializes the audio device for recording\n");

	char * filename = "audio.mp3";
    f = fopen(filename, "wb");
    if (!f) {
      fprintf(stderr, "could not open %s\n", filename);
      exit(1);
  }

  // register all the codecs
  avcodec_init();
  avcodec_register_all();

  AVCodec *codec;

  // find the encoder
  codec = avcodec_find_encoder(CODEC_ID_MP3);
  if (!codec) {
    fprintf(stderr, "codec not found\n");
    exit(1);
  }
  c = avcodec_alloc_context();

  // put sample parameters
  //c->bit_rate = 64000;
  c->sample_fmt = SAMPLE_FMT_S16;
  c->sample_rate = sample_rate;
  c->channels = 1;

  // open the codec
  if( avcodec_open(c, codec) < 0) {
      fprintf(stderr, "could not open codec\n");
      exit(1);
  }

  int outbuf_size = 10000;
  outbuf = malloc(outbuf_size);
}

void audio_segment_copy()
{
  int fd, status;
	int err;
  printf("[A_REC] This function copies the audio segment from sound driver\n");

	// Grabbing size of bytes per frame!
	//int frame_size = snd_pcm_format_width( SND_PCM_FORMAT_S16_LE );
	frame_size = 16;
	sample_rate = 8000;
	length = 5;
	
	printf("Frame size is %d bytes\n", frame_size/8);
  int buf_size = length * sample_rate * frame_size/8;
	buf =  malloc(buf_size); // OMG WTF why does this need more memory?

  // Open the microphone device
  fd = open( "/dev/dsp", O_RDWR );
	printf("reading from interface\n");

  //if ((err = snd_pcm_readi (capture_handle, buf, 64)) != 64) {
  // Set sampling parameters
  if( ioctl( fd, SOUND_PCM_WRITE_BITS, &frame_size ) == -1){
    perror("Write bits failed");
    exit(1);
  }
	if( frame_size != 16)
		printf("Sixteen bits weren't set");
	
	int bytes;
  FILE* file = fopen( "/home/engr/hughes11/Desktop/raw-audio.wav", "wb" );
  if( (bytes = read( fd, buf, buf_size )) != buf_size )
    printf("Wrong number of bytes read: %d\n", bytes);
  
	// int i; for( i = 0; i < count; i++){
		/*if ((err = snd_pcm_readi (capture_handle, buf, 128)) != 128) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
				 snd_strerror (err));
			exit (1);
		}
        printf("buf is %i", buf  );
      int bytes = 0;
	    if( (bytes = read( fd, buf+i, 2 )) != 2 ){
        printf("Bytes: %d\n", bytes);
        perror("Wrong number of bytes");
		}
*/
  //fwrite( buf, buf_size, 1, file );
  //write( fd, buf, buf_size );
  //write( buf, buf_size, 1, file );
  audio_segment_compress();
	fclose(file);
}

void audio_segment_compress()
{
    printf("[A_REC] This function should compress the raw wav to MP3 or AAC\n");
		/* must be called before using avcodec lib */
    int frame_size, out_size, outbuf_size;
    short *samples;

    printf("Audio encoding\n");

    avcodec_init();

    /* register all the codecs */
    avcodec_register_all();
    AVCodec *codec;
    AVCodecContext *c= NULL;
    /* find the MP2 encoder */
    codec = avcodec_find_encoder(CODEC_ID_MP3);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }
    c= avcodec_alloc_context();
    /* put sample parameters */
    c->bit_rate = 64000;
    c->sample_rate = sample_rate;
    c->channels = 1;
    //c->sample_rate = 44100;
    //c->channels = 2;
    c->sample_fmt = SAMPLE_FMT_S16;

    /* open it */
    if (avcodec_open(c, codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }
    /* the codec gives us the frame size, in samples */
    frame_size = c->frame_size;
//    samples = malloc(frame_size * 2 * c->channels);
    outbuf_size = 10000;
    outbuf = malloc(outbuf_size);

    /* encode a single tone sound */
/*
    float t = 0;
	int j = 0;
	int i = 0;    
float tincr = 2 * M_PI * 440.0 / c->sample_rate;
    for(i=0;i<200;i++) {
        for(j=0;j<frame_size;j++) {
            samples[2*j] = (int)(sin(t) * 10000);
            samples[2*j+1] = samples[2*j];
            t += tincr;
        }
        //out_size = avcodec_encode_audio(c, outbuf, outbuf_size, samples);
        out_size = avcodec_encode_audio(c, outbuf, outbuf_size, buf);
        fwrite(outbuf, 1, out_size, f);
    }

	*/
    out_size = avcodec_encode_audio(c, outbuf, sizeof(buf), buf);
    printf("Encoded %d bytes\n", out_size);
    fwrite(outbuf, 1, out_size, f);
}

void audio_exit(){

  if( fclose(f) != 0)
    perror("fclose");
	
  free(outbuf);	
  free(buf);
  avcodec_close(c);
  av_free(c);

	printf("closing interface");
	//snd_pcm_close (capture_handle);
}
