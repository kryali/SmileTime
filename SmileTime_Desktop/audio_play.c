#include "audio_play.h"
#include <alsa/asoundlib.h>
#define ALSA_PCM_NEW_HW_PARAMS_API

snd_pcm_t *handle;
snd_pcm_uframes_t frames;
pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;
int channels;
snd_pcm_hw_params_t *params;

void audio_play_init()
{
  int dir;
  unsigned int val;
  unsigned int sample_rate;
  int rc;

  printf("[A_PLAY] Initializing the sound device\n");

  // Open PCM device for playback.
  rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0)
  {
    fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
    exit(1);
  }

  // Allocate a hardware parameters object.
  snd_pcm_hw_params_alloca(&params);

  // Fill it in with default values.
  snd_pcm_hw_params_any(handle, params);

  // Interleaved mode
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

  // Signed 16-bit little-endian format
  snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

  // Set number of channels. Our project has mono.
  channels = 1;
  snd_pcm_hw_params_set_channels(handle, params, channels);

  // Set sample rate
  sample_rate = 11025;
  snd_pcm_hw_params_set_rate_near(handle, params, &sample_rate, &dir);

  // Set period size to 32 frames.
  frames = sample_rate;
  snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

  // Write the parameters to the driver
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0)
  {
    fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
    exit(1);
  }

  // Use a buffer large enough to hold one period
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  //size = frames * 2* channels; // 2 bytes/sample, 2 channels

  // We want to loop for 5 seconds
  snd_pcm_hw_params_get_period_time(params, &val, &dir);

  printf("[A_PLAY] Sound device initialized. FUCKYEAH!\n");
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
	audioBuffer = malloc(AUDIO_PACKET_SIZE + sizeof(av_packet));
	memset(audioBuffer, 0, AUDIO_PACKET_SIZE + sizeof(av_packet));
}

void * read_audio_packet(){
	int readbytes = 0;
	struct sockaddr_in si;
	unsigned int sLen = sizeof(si);
  int frames = AUDIO_PACKET_SIZE/2/channels; // Each frame is 2 bytes
  int rc;
	memset(audioBuffer, 0, AUDIO_PACKET_SIZE + sizeof(av_packet));
	if( (readbytes = recvfrom(audio_socket, audioBuffer, AUDIO_PACKET_SIZE + sizeof(av_packet), 0, &si, &sLen))== -1){
		perror("recvfrom");
	}
	pthread_mutex_lock(&bytes_received_mutex);
	bytes_received += readbytes;
	pthread_mutex_unlock(&bytes_received_mutex);
  rc = snd_pcm_writei(handle, ((void*)audioBuffer) + sizeof(av_packet), frames);

  /* EPIPE means underrun */
  if (rc == -EPIPE)
  {
//    fprintf(stderr, "underrun occurred\n");
    snd_pcm_prepare(handle);
  }
  else if (rc < 0)
  {
    fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
  }
  else if (rc != (int)frames)
  {
    fprintf(stderr, "short write, write %d frames\n", rc);
  }

	return audioBuffer;
}

void audio_play_close() {
  snd_pcm_drain(handle);
  snd_pcm_close(handle);
}
