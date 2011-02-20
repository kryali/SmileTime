#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#ifndef BUFFERCOUNT
#define BUFFERCOUNT 10
#endif

#ifndef V4L2_PIX_FMT_PJPG
#define V4L2_PIX_FMT_PJPG v4l2_fourcc('P', 'J', 'P', 'G')
#endif

#ifndef V4L2_PIX_FMT_RGB
#define V4L2_PIX_FMT_RGB v4l2_fourcc('G', 'B', 'R', 'G')
#endif

#ifndef V4L2_PIX_FMT_MJPG
#define V4L2_PIX_FMT_MJPG v4l2_fourcc('M', 'J', 'P', 'G')
#endif

#ifndef V4L2_PIX_FMT_JPEG
#define V4L2_PIX_FMT_JPEG v4l2_fourcc('J', 'P', 'E', 'G')
#endif

#ifndef V4L2_CID_PAN_RELATIVE
#define V4L2_CID_PAN_RELATIVE 0x009A0904
#endif

#ifndef V4L2_CID_TILT_RELATIVE
#define V4L2_CID_TILT_RELATIVE 0x009A0905
#endif

#ifndef V4L2_CID_PANTILT_RESET
#define V4L2_CID_PANTILT_RESET 0x0A046D03
#endif

#ifndef V4L2_CID_PAN_RESET
#define V4L2_CID_PAN_RESET 0x009A0906
#endif

#ifndef V4L2_CID_TILT_RESET
#define V4L2_CID_TILT_RESET 0x009A0907
#endif

#define STREAM_FRAME_RATE 13 //frames per second
#define STREAM_PIX_FMT PIX_FMT_YUV420P // Encode to YUV420 pixel format
#define CAMERA_PIX_FMT PIX_FMT_YUYV422 // Read YUYV422 from camera
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480

#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H


AVCodec *video_codec;
AVCodecContext *video_context;
AVStream *video_st;

struct buffer {
    void * start;
    int length;
};

struct buffer * buffers;

pthread_t video_thread_id;
pthread_t audio_thread_id;

int stopRecording;

void video_record_init(AVOutputFormat *fmt, AVFormatContext *oc);
int video_frame_copy();
void video_frame_compress(int bufferIndex);
void video_frame_write();
void video_close();
void add_video_stream(enum CodecID codec_id);
AVFrame *alloc_frame(uint8_t *frame_buf, enum PixelFormat pix_fmt, int width, int height);
void open_video();
void print_Camera_Info();
void mmap_init();
void set_format();
void xioctl(int ctrl, int value);
void pan_relative(int pan);
void tilt_relative(int tilt);
void pan_reset();
void tilt_reset();
void panTilt_reset();
#endif
