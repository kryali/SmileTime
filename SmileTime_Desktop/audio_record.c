#include "audio_record.h"
#include "recorder_server.h"
#include "include.h"

int microphone_fd = -1;
char *microphone_name = "/dev/dsp";

short *audio_buf;
int audio_buf_size;
struct timeb time_of_copy;
struct timeb time_of_send;

av_packet av;

unsigned int channels = 1;
unsigned int sample_bits = 16; // bits per sample
unsigned int sample_rate = 11025; // samples per second
unsigned int sample_size; // size of a sample in bytes
unsigned int audio_input_frame_size = 2080; // number of samples per frame 
// ALSA variables
snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *capture_hw_params;

void audio_record_init()
{ 
  int err;

  // Initialize the ALSA device for recording
  if ((err = snd_pcm_open (&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device \"default\" (%s)\n", snd_strerror (err));
    exit (1);
  }
     
  if ((err = snd_pcm_hw_params_malloc (&capture_hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
    exit (1);
  }
       
  if ((err = snd_pcm_hw_params_any (capture_handle, capture_hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_access (capture_handle, capture_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n", snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_format (capture_handle, capture_hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n", snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, capture_hw_params, &sample_rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n", snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_channels (capture_handle, capture_hw_params, channels)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params (capture_handle, capture_hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror (err));
    exit (1);
  }

  snd_pcm_hw_params_free (capture_hw_params);

  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
    exit (1);
  }

	sample_size = channels * sample_bits/8;

  audio_buf_size = audio_input_frame_size * sample_size; audio_buf = malloc(audio_buf_size);
}

void audio_segment_copy()
{	
	//if( (read( microphone_fd, audio_buf, audio_buf_size )) != audio_buf_size )
	//	perror("audio_segment_copy read: ");
  int frames = 2080;
  int err;
  if( (err = snd_pcm_readi (capture_handle, audio_buf, frames)) != frames )
  {
    printf("Only read %d frames instead of %d", err, frames); 
  }
	ftime(&time_of_copy);

	av.packetType = AUDIO_PACKET;
	av.length = audio_buf_size;
}

void audio_segment_send()
{
	ftime(&time_of_send);
	av.latency = (time_of_send.time * 1000) + time_of_send.millitm - (time_of_copy.time * 1000) - time_of_copy.millitm;
	HTTP_packet* http = av_to_network_packet(&av, audio_buf);
	xwrite(http, video_socket, AUDIO_PORT);
	pthread_mutex_lock(&bytes_sent_mutex);
	bytes_sent += http->length;
	pthread_mutex_unlock(&bytes_sent_mutex);
	destroy_HTTP_packet(http);
}

//Frees all memory and closes codecs.
void audio_close()
{
	free(audio_buf);
	close(microphone_fd);
}
