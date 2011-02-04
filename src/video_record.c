#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

void video_record_init()
{
	// Open up the initial video stream
	int fd;
	if( (fd = open("/dev/video0", O_NONBLOCK)) == -1){
		perror("open");
	}

	// Query the capabilities of the video source
	struct v4l2_capability cap;
	if(ioctl(fd, VIDIOC_QUERYCAP, &cap)==-1){
		perror("ioctl");
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
	if(ioctl(fd, VIDIOC_CROPCAP, &crop)==-1){
		printf("Couldn't get cropping info\n");
		perror("ioctl");
	}
	
	// Grab the default dimensions
	struct v4l2_rect defaultRect;
	defaultRect = crop.defrect;
	printf("Default cropping rectangle\nLeft: %d, Top: %d\n %dpx by %dpx\n", defaultRect.left, defaultRect.top, defaultRect.width, defaultRect.height);


	// Set the format of the image from the video
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = defaultRect.width;
	format.fmt.pix.height = defaultRect.height;
//	format.fmt.pix.pixelformat = V4L2_PIX_FMT_PJPG;

	if(ioctl(fd, VIDIOC_G_FMT, &format) == -1){
		printf("Format not supported\n");
		perror("ioctl");
	}
	struct v4l2_pix_format pix_format;
	pix_format = format.fmt.pix;
	printf("Image Width: %d",pix_format.width);

    //printf("[V_REC] This function initialize the camera device and V4L2 interface\n");
}

void video_frame_copy()
{
    printf("[V_REC] This function copies the raw image from webcam frame buffer to program memory through V4L2 interface\n");
}

void video_frame_compress()
{
    printf("[V_REC] This function should compress the raw image to JPEG image, or MPEG-4 or H.264 frame if you choose to implemente that feature\n");
}
