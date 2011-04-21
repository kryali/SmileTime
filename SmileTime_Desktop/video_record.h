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
#include <sys/timeb.h>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <jpeglib.h>
#include "recorder_server.h"
#include "include.h"
#include "buffer_queue.h"

#ifndef BUFFERCOUNT
#define BUFFERCOUNT 10
#endif

#ifndef V4L2_PIX_FMT_JPEG
#define V4L2_PIX_FMT_JPEG v4l2_fourcc('J', 'P', 'E', 'G')
#endif

#ifndef V4L2_PIX_FMT_YUYV422
#define V4L2_PIX_FMT_YUYV422 v4l2_fourcc('Y', 'U', 'Y', 'V')
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

#ifndef CAMERA_PIX_FMT
#define CAMERA_PIX_FMT V4L2_PIX_FMT_YUYV422 //read from camera
#endif

#ifndef VIDEO_WIDTH
#define VIDEO_WIDTH 320
#endif

#ifndef VIDEO_HEIGHT
#define VIDEO_HEIGHT 240
#endif

#define OUTBUFFER_SIZE 100000

#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H


int camera_fd;
BufferQueue *videoq;

struct Buffer * buffers;

struct timeb currentTime;
struct timeb startTime;

int stopRecording;
float framesps;

void video_record_init();
void video_compress_init();
int video_frame_copy();
void video_frame_compress(int bufferIndex);
void video_frame_queue();
void video_frame_write();
void video_close();
void print_Camera_Info();
void mmap_init();
void set_camera_output_format();

void init_destination(j_compress_ptr cinfo);
boolean empty_output_buffer(j_compress_ptr cinfo);
void term_destination(j_compress_ptr cinfo);

#endif
