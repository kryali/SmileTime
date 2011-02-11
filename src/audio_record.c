#include "audio_record.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>

// http://www.equalarea.com/paul/alsa-audio.html
int microphone_fd = -1;
char* microphone_name = "default";

void audio_record_init()
{
		int i;
		int err;
		short buf[128];
		snd_pcm_t *capture_handle;
		snd_pcm_hw_params_t *hw_params;
		int rate = 44100;
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
		printf("reading from interface\n");
		for (i = 0; i < 10; ++i) {
			printf("%d\n",i);
			if ((err = snd_pcm_readi (capture_handle, buf, 128)) != 128) {
				fprintf (stderr, "read from audio interface failed (%s)\n",
					 snd_strerror (err));
				exit (1);
			}
		}
		printf("closing interface");
		snd_pcm_close (capture_handle);
		exit (0);

}

void audio_segment_copy()
{
    printf("[A_REC] This function copies the audio segment from sound driver\n");
}

void audio_segment_compress()
{
    printf("[A_REC] This function should compress the raw wav to MP3 or AAC\n");
}
