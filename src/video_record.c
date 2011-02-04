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
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

#ifndef V4L2_PIX_FMT_PJPG
#define V4L2_PIX_FMT_PJPG v4l2_fourcc('P', 'J', 'P', 'G')
#endif


//SOURCES:
/*
http://v4l2spec.bytesex.org/spec/book1.htm
http://v4l2spec.bytesex.org/spec/capture-example.html
*/

int camera_fd = -1;
char* camera_name = "/dev/video";
size_t frame_size;
char* frame_buffer;

//This function initialize the camera device and V4L2 interface
void video_record_init()
{
	//open camera
	camera_fd = open(camera_name, O_RDWR | O_NONBLOCK);
	if(camera_fd == -1)
	{
		printf("error opening camera %s\n", camera_name);
		return;
	}

	//find capabilities of camera
	struct v4l2_capability cap;
	ioctl(camera_fd, VIDIOC_QUERYCAP, &cap);
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
                fprintf (stderr, "%s is not a camera\n", camera_name);
                return;
        }

	// Print out basic statistics
	printf("Driver: %s\n", cap.driver);
	printf("Device: %s\n", cap.card);
	printf("bus_info: %s\n", cap.bus_info);
	if( !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ){
		printf("No video capture capabilities!\n");
	}

	// Get information about the video cropping and scaling abilities
	struct v4l2_cropcap crop;
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // Set the cropping request to be specific to video capture
	if(ioctl(camera_fd, VIDIOC_CROPCAP, &crop)==-1){
		printf("Couldn't get cropping info\n");
		perror("ioctl");
	}
	
	// Grab the default dimensions
	struct v4l2_rect defaultRect;
	defaultRect = crop.bounds;
	printf("Default cropping rectangle\nLeft: %d, Top: %d\n %dpx by %dpx\n", defaultRect.left, defaultRect.top, defaultRect.width, defaultRect.height);


	// Set the format of the image from the video
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = defaultRect.width;
	format.fmt.pix.height = defaultRect.height;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_PJPG;

	if(ioctl(camera_fd, VIDIOC_G_FMT, &format) == -1){
		printf("Format not supported\n");
		perror("ioctl");
	}
	struct v4l2_pix_format pix_format;
	pix_format = format.fmt.pix;
	printf("Image Width: %d\n",pix_format.width);

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

	//somehow find frame size from camera?
	frame_size = 1024;//not sure what this should be
	frame_buffer = malloc(frame_size);

	video_frame_copy();
    //printf("[V_REC] This function initialize the camera device and V4L2 interface\n");
}

//This function copies the raw image from webcam frame buffer to program memory through V4L2 interface
void video_frame_copy()
{
	if( read(camera_fd, frame_buffer, frame_size)==-1){
		perror("read");
	}
}

//This function should compress the raw image to JPEG image, or MPEG-4 or H.264 frame if you choose to implemente that feature
void video_frame_compress()
{
	
}

//Closes the camera and frees all memory
void video_close()
{
	int closed = close(camera_fd);
	if(closed == 0)
	{
		printf("closed: %d\n", camera_fd);
		camera_fd = -1;
		free(frame_buffer);
	}
	else
	{
		printf("error closing: %d\n", camera_fd);
	}
}
