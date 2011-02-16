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
    AVCodecContext *c= NULL;


void audio_record_init()
{
		printf("[A_REC] This function initializes the audio device for recording\n");
		int i;
		int err;
		unsigned int rate = 44100;
		snd_pcm_hw_params_t *hw_params;
		if ((err = snd_pcm_open (&capture_handle, microphone_name, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
			fprintf (stderr, "cannot open audio device %s (%s)\n", 
				 microphone_name,
				 snd_strerror (err));
			exit (1);
		}

		if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
			fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				 snd_strerror (err));
			exit (1);
		}

		if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
			fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				 snd_strerror (err));
			exit (1);
		}

		if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
			fprintf (stderr, "cannot set access type (%s)\n",
				 snd_strerror (err));
			exit (1);
		}

		if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
			fprintf (stderr, "cannot set sample format (%s)\n",
				 snd_strerror (err));
			exit (1);
		}

		if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
			fprintf (stderr, "cannot set sample rate (%s)\n",
				 snd_strerror (err));
			exit (1);
		}

		if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
			fprintf (stderr, "cannot set channel count (%s)\n",
				 snd_strerror (err));
			exit (1);
		}

		if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
			fprintf (stderr, "cannot set parameters (%s)\n",
				 snd_strerror (err));
			exit (1);
		}

		snd_pcm_hw_params_free (hw_params);

		if ((err = snd_pcm_prepare (capture_handle)) < 0) {
			fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				 snd_strerror (err));
			exit (1);
		}

    //f = fopen("audio.mp2", "w+");
	char * filename = "audio.mp3";
    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "could not open %s\n", filename);
        exit(1);
    }

}

void audio_segment_copy()
{
    int fd, status;
	int err;
    printf("[A_REC] This function copies the audio segment from sound driver\n");

	// Grabbing size of bytes per frame!
	int frame_size = snd_pcm_format_width( SND_PCM_FORMAT_S16_LE );
	
	printf("Frame size is %d\n", frame_size/8);
	buf =  malloc(2 * 12800 * frame_size/8); // OMG WTF why does this need more memory?

    fd = open( "/dev/dsp", O_RDWR );
	printf("reading from interface\n");
		//if ((err = snd_pcm_readi (capture_handle, buf, 64)) != 64) {
    printf("fd = %d\n", fd);
    int arg = 16;
    if( ioctl( fd, SOUND_PCM_WRITE_BITS, &arg ) == -1){
		perror("Write bits failed");
		exit(1);
	}
	if( arg != 16)
		printf("Sixteen bits weren't set");
	
	int i;
	int count = 44400/128;
	for( i = 0; i < count; i++){
		/*if ((err = snd_pcm_readi (capture_handle, buf, 128)) != 128) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
				 snd_strerror (err));
			exit (1);
		}
*/
	int bytes = 0;
	    if( (bytes = read( fd, buf, 256 )) != 256 ){
			printf("Bytes: %d\n", bytes);
           perror("Wrong number of bytes");
		}
        

		audio_segment_compress();
	}
}

void audio_segment_compress()
{
    printf("[A_REC] This function should compress the raw wav to MP3 or AAC\n");
		/* must be called before using avcodec lib */
	avcodec_init();

	/* register all the codecs */
	avcodec_register_all();
    int frame_size, out_size, outbuf_size;
    short *samples;



    printf("Audio encoding\n");

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
    c->sample_rate = 44100;
    c->channels = 2;
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
    out_size = avcodec_encode_audio(c, outbuf, 256 , buf);
	printf("Encoded %d bytes\n", out_size);
    fwrite(outbuf, 1, 256, f);
}

void audio_exit(){

if( fclose(f) != 0)
	perror("fclose");
	
    free(outbuf);	
    free(buf);
    avcodec_close(c);
    av_free(c);

	printf("closing interface");
	snd_pcm_close (capture_handle);
}
