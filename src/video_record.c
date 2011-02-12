#include "video_record.h"

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

#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#ifndef V4L2_PIX_FMT_PJPG
#define V4L2_PIX_FMT_PJPG v4l2_fourcc('P', 'J', 'P', 'G')
#endif

#ifndef V4L2_PIX_FMT_MJPG
#define V4L2_PIX_FMT_MJPG v4l2_fourcc('M', 'J', 'P', 'G')
#endif


//SOURCES:
/*
http://v4l2spec.bytesex.org/spec/book1.htm
http://v4l2spec.bytesex.org/spec/capture-example.html
*/

int camera_fd = -1;
char* camera_name = "/dev/video1";

struct buffer {
	void * start;
	size_t length;
};

struct buffer * buffers = NULL;

void print_default_crop(){
	// Get information about the video cropping and scaling abilities
	struct v4l2_cropcap crop;
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // Set the cropping request to be specific to video capture
	if(ioctl(camera_fd, VIDIOC_CROPCAP, &crop)==-1){
		printf("Couldn't get cropping info\n");
		perror("ioctl");
	}
	struct v4l2_rect defaultRect;
	defaultRect = crop.bounds;
	printf("Default cropping rectangle\nLeft: %d, Top: %d\n %dpx by %dpx\n", defaultRect.left, defaultRect.top, defaultRect.width, defaultRect.height);

	struct v4l2_fmtdesc fmtdesc;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(camera_fd, VIDIOC_ENUM_FMT, &fmtdesc)==-1){
		printf("Format query failed\n");
		perror("ioctl");
	}
	printf("Format: %s\n", fmtdesc.description);
}

void print_input_info(){
	//found these online
	struct v4l2_input input;
	int index;

	if (-1 == ioctl (camera_fd, VIDIOC_G_INPUT, &index)) {
       		perror ("VIDIOC_G_INPUT");
        	exit (EXIT_FAILURE);
	}

	memset (&input, 0, sizeof (input));
	input.index = index;

	if (-1 == ioctl (camera_fd, VIDIOC_ENUMINPUT, &input)) {
   	     perror ("VIDIOC_ENUMINPUT");
  	      exit (EXIT_FAILURE);
	}

	printf ("Current input: %s\n", input.name);

}

void set_format(){
	// Set the format of the image from the video
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = 640;
	format.fmt.pix.height = 480;
	format.fmt.pix.field = V4L2_FIELD_INTERLACED;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	if(ioctl(camera_fd, VIDIOC_S_FMT, &format) == -1){
		printf("Format not supported\n");
		perror("ioctl");
	}
	struct v4l2_pix_format pix_format;
	pix_format = format.fmt.pix;
	printf("Image Width: %d\n",pix_format.width);
}

//This function initialize the camera device and V4L2 interface
void video_record_init(){

	//open camera
	camera_fd = open(camera_name, O_RDWR );
	if(camera_fd == -1){
		printf("error opening camera %s\n", camera_name);
		return;
	}

	print_Camera_Info();

	set_format();	

	mmap_init();	

	read_frame();
    //printf("[V_REC] This function initialize the camera device and V4L2 interface\n");

  encode_frame("/home/mark/Desktop/out.webm");
}

void encode_frame(const char *filename)
{
   AVCodec *codec;
   AVCodecContext *c= NULL;
   int i, out_size, size, x, y, outbuf_size;
   FILE *f;
   AVFrame *picture;
   uint8_t *outbuf, *picture_buf;

   printf("Video encoding\n");

   /* find the mpeg1 video encoder */
   codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
   if (!codec) {
       fprintf(stderr, "codec not found\n");
       exit(1);
   }

   c= avcodec_alloc_context();
   picture= avcodec_alloc_frame();

   /* put sample parameters */
   c->bit_rate = 400000;
   /* resolution must be a multiple of two */
   c->width = 640;
   c->height = 480;
   /* frames per second */
   c->time_base= (AVRational){1,25};
   c->gop_size = 10; /* emit one intra frame every ten frames */
   c->max_b_frames=1;
   c->pix_fmt = PIX_FMT_YUV420P;

   /* open it */
   if (avcodec_open(c, codec) < 0) {
       fprintf(stderr, "could not open codec\n");
       exit(1);
   }

   f = fopen(filename, "wb");
   if (!f) {
       fprintf(stderr, "could not open %s\n", filename);
       exit(1);
   }

   /* alloc image and output buffer */
   outbuf_size = 100000;
   outbuf = malloc(outbuf_size);
   size = c->width * c->height;
   picture_buf = malloc((size * 3) / 2); /* size for YUV 420 */

   picture->data[0] = picture_buf;
   picture->data[1] = picture->data[0] + size;
   picture->data[2] = picture->data[1] + size / 4;
   picture->linesize[0] = c->width;
   picture->linesize[1] = c->width / 2;
   picture->linesize[2] = c->width / 2;

   /* encode 1 second of video */
   for(i=0;i<25;i++) {
       fflush(stdout);
       /* prepare a dummy image */
       /* Y */
       for(y=0;y<c->height;y++) {
           for(x=0;x<c->width;x++) {
               picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
           }
       }

       /* Cb and Cr */
       for(y=0;y<c->height/2;y++) {
           for(x=0;x<c->width/2;x++) {
               picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
               picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
           }
       }

       /* encode the image */
       out_size = avcodec_encode_video(c, outbuf, outbuf_size, picture);
       printf("encoding frame %3d (size=%5d)\n", i, out_size);
       fwrite(outbuf, 1, out_size, f);
   }

   /* get the delayed frames */
   for(; out_size; i++) {
       fflush(stdout);

       out_size = avcodec_encode_video(c, outbuf, outbuf_size, NULL);
       printf("write frame %3d (size=%5d)\n", i, out_size);
       fwrite(outbuf, 1, out_size, f);
   }

   /* add sequence end code to have a real mpeg file */
   outbuf[0] = 0x00;
   outbuf[1] = 0x00;
   outbuf[2] = 0x01;
   outbuf[3] = 0xb7;
   fwrite(outbuf, 1, 4, f);
   fclose(f);
   free(picture_buf);
   free(outbuf);

   avcodec_close(c);
   av_free(c);
   av_free(picture);
   printf("\n");
}

void read_frame(){

	// DEQUEUE frame from buffer
	struct v4l2_buffer buf;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if( ioctl(camera_fd, VIDIOC_DQBUF, &buf) == -1){
		perror("VIDIOC_DQBUF");
	}
	
	printf("Read buffer index:%d\n", buf.index);

	// WRITE buffer to file
	FILE * frame_fd = fopen( "1.jpg", "w+");
	if( fwrite(buffers[buf.index].start, buffers[buf.index].length, 1, frame_fd) != 1)
		perror("fwrite");
	fclose(frame_fd);

	// ENQUEUE frame into buffer
	struct v4l2_buffer bufQ;
	bufQ.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufQ.memory = V4L2_MEMORY_MMAP;
	bufQ.index = buf.index;
	if( ioctl(camera_fd, VIDIOC_QBUF, &bufQ) == -1 ){
		perror("VIDIOC_QBUF");
	}
}

void mmap_init(){
	// Request that the device start using the buffers
	// - Find the number of support buffers
	struct v4l2_requestbuffers reqbuf;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.count = 5;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if (ioctl (camera_fd, VIDIOC_REQBUFS, &reqbuf) == -1){
		perror("ioctl");
		exit(EXIT_FAILURE);
	}	
	printf("Buffer count: %d\n", reqbuf.count);
	
	buffers = malloc( reqbuf.count * sizeof(*buffers));
	int i =0;

	// Mmap memory for each slice of the buffer	
	for(; i < reqbuf.count; i++){
		struct v4l2_buffer buf;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if( ioctl ( camera_fd, VIDIOC_QUERYBUF, &buf) == -1 ){
			perror("VIDIOC_QUERYBUF");
		}

		printf("Index: %d, Buffer offset: %d\n", i, buf.m.offset);
		buffers[i].length = buf.length; 
		buffers[i].start = mmap( NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, camera_fd, buf.m.offset);
		if (buffers[i].start == MAP_FAILED){
			perror("mmap");
		}
	}
	
	// Start capturing
	for(i=0; i <reqbuf.count; i++){
		struct v4l2_buffer buf;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if( ioctl(camera_fd, VIDIOC_QBUF, &buf) == -1 ){
			perror("VIDIOC_QBUF");
		}
	}

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(camera_fd, VIDIOC_STREAMON, &type) == -1){
		perror("VIDIOC_STREAMON");
	}
	

}

//This function copies the raw image from webcam frame buffer to program memory through V4L2 interface
void video_frame_copy(){

}

//This function should compress the raw image to JPEG image, or MPEG-4 or H.264 frame if you choose to implemente that feature
void video_frame_compress(){
	
}

//Closes the camera and frees all memory
void video_close(){
	printf("Closing stream");
	int closed = close(camera_fd);
	if(closed == 0){
		printf("closed: %d\n", camera_fd);
		camera_fd = -1;
	}
	else{
		printf("error closing: %d\n", camera_fd);
	}
}

//find capabilities of camera and print them out
void print_Camera_Info(){
	struct v4l2_capability cap;
	ioctl(camera_fd, VIDIOC_QUERYCAP, &cap);
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
                fprintf (stderr, "%s is not a camera\n", camera_name);
                return;
        }
	// Print out basic statistics
	printf("File Descriptor: %d\n", camera_fd);
	printf("Driver: %s\n", cap.driver);
	printf("Device: %s\n", cap.card);
	printf("bus_info: %s\n", cap.bus_info);
	if( !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ){
		printf("No video capture capabilities!\n");
	}
	if( !(cap.capabilities & V4L2_CAP_READWRITE) ){
		printf("No read/write capabilities!\n");
	}
	if( !(cap.capabilities & V4L2_CAP_STREAMING) ){
		printf("No streaming capabilities!\n");
	}
}
