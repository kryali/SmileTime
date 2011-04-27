#include "video_record.h"

//SOURCES:
/*
http://v4l2spec.bytesex.org/spec/book1.htm
http://v4l2spec.bytesex.org/spec/capture-example.html
*/
char* camera_name = "/dev/video0";
int width = VIDEO_WIDTH;
int height = VIDEO_HEIGHT;

av_packet av;
char* jpegStart;
struct timeb time_of_copy;

int bufferIndex;

//This function initialize the camera device and V4L2 interface
void video_record_init(){
	//open camera
	camera_fd = open(camera_name, O_RDWR );
	if(camera_fd == -1){
		printf("error opening camera %s\n", camera_name);
		return;
	}

	set_camera_output_format();
	buffers = NULL;
	decompressed_frame_camera = NULL;
	mmap_init();
}

//This function copies the raw image from webcam frame buffer to program memory through V4L2 interface
void video_frame_copy()
{
	// DEQUEUE frame from buffer
	struct v4l2_buffer buf;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if( ioctl(camera_fd, VIDIOC_DQBUF, &buf) == -1){
		perror("VIDIOC_DQBUF");
	}

	// ENQUEUE frame into buffer
	struct v4l2_buffer bufQ;
	bufQ.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufQ.memory = V4L2_MEMORY_MMAP;
	bufQ.index = buf.index;
	if( ioctl(camera_fd, VIDIOC_QBUF, &bufQ) == -1 ){
		perror("VIDIOC_QBUF");
	}
	ftime(&time_of_copy);
	bufferIndex = bufQ.index;

	int jpegSize = mjpeg2Jpeg(&jpegStart, buffers[bufferIndex].start, buf.bytesused);
	av.packetType = VIDEO_PACKET;
	av.length = jpegSize;
	av.timestamp = (time_of_copy.time * 1000) + time_of_copy.millitm;
}

// This function should decompress the captured MJPG image to a yuv image.
void video_frame_mjpg_to_yuv()
{
	free(jpegStart);
	//if(decompressed_frame_camera != NULL){
		//free(decompressed_frame_camera);
	//}
	if(jpeg_decode(&decompressed_frame_camera, buffers[bufferIndex].start, &width, &height) < 0){
		printf("jpeg decode failure\n");
		exit(1);
	}
}

void video_frame_send()
{
	printf("video frame send\n");
	HTTP_packet* http = av_to_network_packet(&av, jpegStart);
	xwrite(http, video_socket);
	destroy_HTTP_packet(http);
}

//Closes the camera and frees all memory
void video_close(){
	int closed = close(camera_fd);
	if(closed == 0)
		camera_fd = -1;
}

void mmap_init(){
	// Request that the device start using the buffers
	// - Find the number of support buffers
	struct v4l2_requestbuffers reqbuf;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.count = BUFFERCOUNT;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if (ioctl (camera_fd, VIDIOC_REQBUFS, &reqbuf) == -1){
		perror("ioctl");
		exit(EXIT_FAILURE);
	}	
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
		//printf("Index: %d, Buffer offset: %d\n", i, buf.m.offset);
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

void set_camera_output_format(){
	// Set the format of the image from the video device
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = VIDEO_WIDTH;
	format.fmt.pix.height = VIDEO_HEIGHT;
	format.fmt.pix.field = V4L2_FIELD_INTERLACED;
	format.fmt.pix.pixelformat = CAMERA_PIX_FMT;

	if(ioctl(camera_fd, VIDIOC_S_FMT, &format) == -1){
		perror("VIDIOC_S_FMT");
		exit( EXIT_FAILURE );
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
	printf("Driver: %s\n", cap.driver);
	printf("Device: %s\n", cap.card);
	printf("bus_info: %s\n", cap.bus_info);
	if( !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ){
		printf("No video capture capabilities!\n");
	}
	if( !(cap.capabilities & V4L2_CAP_READWRITE) ){
		printf("No read/write capabilities! (this is okay, we don't use it.)\n");
	}
	if( !(cap.capabilities & V4L2_CAP_STREAMING) ){
		printf("No streaming capabilities!\n");
	}
}

