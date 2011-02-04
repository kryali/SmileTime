#include <stdio.h>
#include <sys/ioctl.h>

void video_record_init()
{
	int fd;
	struct v4l2_capability;
	if(ioctl(fd, VIDIOC_QUERYCAP, &v4l2_capability)==-1){
		perror("ioctl");
	}
    //kkkoprintf("[V_REC] This function initialize the camera device and V4L2 interface\n");
}

void video_frame_copy()
{
    printf("[V_REC] This function copies the raw image from webcam frame buffer to program memory through V4L2 interface\n");
}

void video_frame_compress()
{
    printf("[V_REC] This function should compress the raw image to JPEG image, or MPEG-4 or H.264 frame if you choose to implemente that feature\n");
}
