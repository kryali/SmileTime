#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>

#include "buffer_queue.h"

#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

BufferQueue *audioq;

void audio_record_init();
void audio_segment_copy();
void audio_segment_queue();
void audio_segment_send();
void audio_close();
void open_audio();
#endif
