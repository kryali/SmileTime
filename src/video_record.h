#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

struct buffer {
    void * start;
    int length;
};

struct buffer * buffers;

void video_record_init();
int video_frame_copy();
void video_frame_compress();
void video_close();
void print_Camera_Info();
void mmap_init();
void print_default_crop();
void print_input_info();
void set_format();
void read_frame();
void encode_frame( const char *filename, int index);

#endif
