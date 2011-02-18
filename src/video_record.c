#include "video_record.h"

//SOURCES:
/*
http://v4l2spec.bytesex.org/spec/book1.htm
http://v4l2spec.bytesex.org/spec/capture-example.html
*/

int camera_fd = -1;
char* camera_name = "/dev/video0";
uint8_t *outbuf, *inbuf;
int outbuf_size, inbuf_size;
int enc_size;

//This function initialize the camera device and V4L2 interface
void video_record_init(){
	//open camera
	camera_fd = open(camera_name, O_RDWR );
	if(camera_fd == -1){
		printf("error opening camera %s\n", camera_name);
		return;
	}
	   // find the h264 video encoder
   video_codec = avcodec_find_encoder(CODEC_ID_H264);
   if (!video_codec) {
       fprintf(stderr, "video_codec not found\n");
       exit(1);
       
   }

   video_context = avcodec_alloc_context();

   // sample parameters
   video_context->bit_rate = 400000;
   video_context->width = 640;
   video_context->height = 480;

   // frames parameters
   video_context->time_base= (AVRational){1,25};
   video_context->gop_size = 10; // emit one intra frame every ten frames
   video_context->max_b_frames=1;
   video_context->pix_fmt = PIX_FMT_YUV420P;

   // h264 parameters
   video_context->me_range = 16;
   video_context->max_qdiff = 4;
   video_context->qmin = 10;
   video_context->qmax = 51;
   video_context->qcompress = 0.6;
   
   // open the codec
   if (avcodec_open(video_context, video_codec) < 0) {
       fprintf(stderr, "could not open video_codec\n");
       exit(1);
   }
   
   outbuf_size = avpicture_get_size(PIX_FMT_YUV420P, video_context->width, video_context->height);
   outbuf = av_malloc(outbuf_size);
   inbuf_size = avpicture_get_size(PIX_FMT_YUYV422, video_context->width, video_context->height);
   inbuf = av_malloc(inbuf_size);
   
	print_Camera_Info();
	set_format();	
	mmap_init();	
  //printf("[V_REC] This function initialize the camera device and V4L2 interface\n");
}

//This function copies the raw image from webcam frame buffer to program memory through V4L2 interface
void video_frame_copy(){

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
}

// This function should compress the raw image to JPEG image, or MPEG-4 or H.264 frame if you choose to implemente that feature
void video_frame_compress(){
	printf("TROL\n");
   int i, x;
   i = 0;

   AVFrame *srcFrame, *dstFrame; 

   static struct SwsContext *img_convert_ctx;

   // Allocate space for the frames
   srcFrame = avcodec_alloc_frame(); 
   dstFrame = avcodec_alloc_frame(); 
	printf("TROL2\n");

   // Create AVFrame for YUV420 frame
   avpicture_fill((AVPicture *)dstFrame, outbuf, PIX_FMT_YUV420P, video_context->width, video_context->height);

	printf("TRO5\n");
   // Create AVFrame for YUYV422 frame
   x = avpicture_fill((AVPicture *)srcFrame, buffers[0].start, PIX_FMT_YUYV422, video_context->width, video_context->height);

   // Make the conversion context
   img_convert_ctx = sws_getContext(video_context->width, video_context->height, 
                    PIX_FMT_YUYV422, 
                    video_context->width, video_context->height, PIX_FMT_YUV420P, SWS_BICUBIC, 
                    NULL, NULL, NULL);

	printf("TROL9\n");
   // Convert the image to YUV420
   if(srcFrame->data == NULL){
   	printf("srcFrame is null");
   	exit(1);
	}printf("TROL19\n");	
   if(dstFrame->data == NULL){
   	printf("dstFrame is null");
   	exit(1);
	}	printf("TROL29\n");
   
   sws_scale(img_convert_ctx, srcFrame->data, 
         srcFrame->linesize, 0, 
         video_context->height, 
         dstFrame->data, dstFrame->linesize);

	printf("TROL15\n");
   // Encode the frame
   do { 
     // Why so many empty encodes at the beginning?
     enc_size = avcodec_encode_video(video_context, outbuf, outbuf_size, dstFrame);
     // printf("encoding frame %3d (size=%5d)\n", i, enc_size);
     //fwrite(outbuf, 1, enc_size, video_file);
   } while ( enc_size == 0 );

   /*// Get the delayed frames
   //for(; enc_size; i++) {
   for(; i<=5; i++) {
     // Why is this necessary?
     fflush(stdout);
     enc_size = avcodec_encode_video(video_context, outbuf, outbuf_size, NULL);
     // printf("write frame %3d (size=%5d)\n", i, enc_size);
     if( enc_size ) fwrite(outbuf, 1, enc_size, video_file);
   }*/
}

//Closes the camera and frees all memory
void video_close(){
	av_free(outbuf);
   av_free(inbuf);
   avcodec_close(video_context);
   av_free(video_context);
	int closed = close(camera_fd);
	if(closed == 0)
		camera_fd = -1;
}


void read_frame(){

	// DEQUEUE frame from buffer
	struct v4l2_buffer buf;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if( ioctl(camera_fd, VIDIOC_DQBUF, &buf) == -1){
		perror("VIDIOC_DQBUF");
	}
	
	//printf("Read buffer index:%d\n", buf.index);

	// WRITE buffer to file
	/*FILE * frame_fd = fopen( "1.jpg", "w+");
	if( fwrite(buffers[buf.index].start, buffers[buf.index].length, 1, frame_fd) != 1)
		perror("fwrite");
	fclose(frame_fd);
  */

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
	reqbuf.count = BUFFERCOUNT;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if (ioctl (camera_fd, VIDIOC_REQBUFS, &reqbuf) == -1){
		perror("ioctl");
		exit(EXIT_FAILURE);
	}	
	//printf("Buffer count: %d\n", reqbuf.count);
	
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

	int i = 0;
	for(; i < 5; i++){
		struct v4l2_fmtdesc fmtdesc;
		fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmtdesc.index = 1;
		if(ioctl(camera_fd, VIDIOC_ENUM_FMT, &fmtdesc)==-1){
			printf("Format query failed\n");
			perror("ioctl");
		}
		printf("Format: %s\n", fmtdesc.description);
	}
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
//	format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPG;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	if(ioctl(camera_fd, VIDIOC_S_FMT, &format) == -1){
		perror("VIDIOC_S_FMT");
		exit( EXIT_FAILURE );
	}
	struct v4l2_pix_format pix_format;
	pix_format = format.fmt.pix;
	printf("Image Width: %d\n",pix_format.width);
	if(!(format.fmt.pix.pixelformat & V4L2_PIX_FMT_MJPG)){
		printf("Error: MJPG compressions wasn't set\n");
	}
}

//http://www.zerofsck.org/2009/03/09/example-code-pan-and-tilt-your-logitech-sphere-webcam-using-python-module-lpantilt-linux-v4l2/
int pan_relative(int pan)
{
    struct v4l2_ext_control xctrls;
    struct v4l2_ext_controls ctrls;
    xctrls.id = V4L2_CID_PAN_RELATIVE;
    xctrls.value = pan;
    ctrls.count = 1;
    ctrls.controls = &xctrls;
    if ( ioctl(camera_fd, VIDIOC_S_EXT_CTRLS, &ctrls) < 0 ) {
        perror("VIDIOC_S_EXT_CTRLS - Pan error. Are the extended controls available?\n");
        return -1;
    } else {
        printf("PAN Success");
    }
    return 0;
}

int tilt_relative(int tilt)
{
    struct v4l2_ext_control xctrls;
    struct v4l2_ext_controls ctrls;
    xctrls.id = V4L2_CID_TILT_RELATIVE;
    xctrls.value = tilt;
    ctrls.count = 1;
    ctrls.controls = &xctrls;
    if ( ioctl(camera_fd, VIDIOC_S_EXT_CTRLS, &ctrls) < 0 )
    {
        perror("VIDIOC_S_EXT_CTRLS - Tilt error. Are the extended controls available?\n");
        return -1;
    } else {
        printf("TILT Success");
    }

    return 0;
}

int panTilt_relative(int pan, int tilt)
{
    struct v4l2_ext_control xctrls[2];
    struct v4l2_ext_controls ctrls;
    xctrls[0].id = V4L2_CID_PAN_RELATIVE;
    xctrls[0].value = pan;
    xctrls[1].id = V4L2_CID_TILT_RELATIVE;
    xctrls[1].value = tilt;
    ctrls.count = 2;
    ctrls.controls = xctrls;
    if ( ioctl(camera_fd, VIDIOC_S_EXT_CTRLS, &ctrls) < 0 )
    {
        perror("VIDIOC_S_EXT_CTRLS - Pan/Tilt error. Are the extended controls available?\n");
        return -1;
    } else {
        printf("PAN/TILT Success");
    }

    return 0;
}

int panTilt_reset()
{
    struct v4l2_ext_control xctrls;
    struct v4l2_ext_controls ctrls;
    xctrls.id = V4L2_CID_PANTILT_RESET;
    xctrls.value = 1;
    ctrls.count = 1;
    ctrls.controls = &xctrls;
    if ( ioctl(camera_fd, VIDIOC_S_EXT_CTRLS, &ctrls) < 0 )
    {
        perror("VIDIOC_S_EXT_CTRLS - Pan/Tilt error. Are the extended controls available?\n");
        return -1;
    } else {
        printf("PAN/TILT reset Success");
    }
	 
    return 0;
}
