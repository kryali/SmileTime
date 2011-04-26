#include "include.h"
#ifndef AUDIO_PLAY_H
#define AUDIO_PLAY_H

#include <stdio.h>

#define AUDIO_PACKET_SIZE 4160

int audio_socket;

void audio_play_init();
void audio_segment_decompress();
void audio_segment_playback();
void audio_play_close();

void init_udp_audio();
void* read_audio_packet();

#endif
