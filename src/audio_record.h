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

#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

AVCodec *audio_codec;
AVCodecContext *audio_context;

void audio_record_init();
void audio_segment_copy();
void audio_segment_compress(AVStream *st);
void audio_segment_write(AVFormatContext *oc);
void audio_close(AVFormatContext *oc);
AVStream *add_audio_stream(AVFormatContext *oc, enum CodecID codec_id);
void open_audio(AVFormatContext *oc);
#endif
