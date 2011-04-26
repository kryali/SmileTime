#include <stdio.h>

char* audio_device;
int bits_per_sample = 16;

void audio_play_init()
{
  // Set audio device to play to
  audio_device = "/dev/dsp";

  // Open the audio device
  open(audio_device, O_WRONLY | O_APPEND);
  printf("[A_PLAY] This function initiates the sound device that plays audio\n");
}

// Do we need this for MP3? Hopefully Android can capture .wave and we can just write() that bia-bia to the sound device
void audio_segment_decompress()
{
  printf("[A_PLAY] This function decompresses the audio data\n");
}

void audio_segment_playback( char* sample )
{
  printf("[A_PLAY] This function plays the sound out to devices\n");

  // Play the packet
  write( audio_device, *sample, bits_per_sample );
}
