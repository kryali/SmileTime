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
#define BUFFERCOUNT 1
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
//#define V4L2 _CID_PAN_RELATIVE (V4L2 _CID_PRIVATE_BASE+7) 
#define V4L2_CID_PAN_RELATIVE 0x009A0904 
#endif

#ifndef V4L2_CID_TILT_RELATIVE 
//#define V4L2 _CID_TILT_RELATIVE (V4L2 _CID_PRIVATE_BASE+8) 
#define V4L2_CID_TILT_RELATIVE 0x009A0905 
#endif

#ifndef V4L2_CID_PANTILT_RESET 
//#define V4L2 _CID_PANTILT_RESET (V4L2 _CID_PRIVATE_BASE+9) 
#define V4L2_CID_PANTILT_RESET 0x0A046D03 
#endif

#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H




AVCodec *video_codec;
AVCodecContext *video_context;


struct buffer {
    void * start;
    int length;
};

struct buffer * buffers;


void video_record_init();
void video_frame_copy();
//void* video_frame_compress();
void video_frame_compress();
void video_close();
void print_Camera_Info();
void mmap_init();
void print_default_crop();
void print_input_info();
void set_format();
void read_frame();
int pan_relative(int pan);
int tilt_relative(int tilt);
int panTilt_relative(int pan, int tilt);
int panTilt_reset();
#endif
