#include "video_record.h"

//SOURCES:
/*
http://v4l2spec.bytesex.org/spec/book1.htm
http://v4l2spec.bytesex.org/spec/capture-example.html
*/
#define STREAM_FRAME_RATE 25 //frames per second
#define STREAM_PIX_FMT PIX_FMT_YUV420P //YUV420 pixel format
#define VIDEO_WIDTH 640;
#define VIDEO_HEIGHT 480;

int camera_fd = -1;
char* camera_name = "/dev/video0";
int enc_size;

AVStream *video_st;
AVFormatContext *output_context;

AVFrame *picture, *tmp_picture;
uint8_t *video_outbuf;
int frame_count, video_outbuf_size;

//This function initialize the camera device and V4L2 interface
void video_record_init(AVFormatContext *oc){
	video_st = NULL;
	output_context = oc;
	//open camera
	camera_fd = open(camera_name, O_RDWR );
	if(camera_fd == -1){
		printf("error opening camera %s\n", camera_name);
		return;
	}
         
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
   int i, x;
   i = 0;
   static struct SwsContext *img_convert_ctx;
   
   uint8_t *outbuf, *inbuf;
	int outbuf_size, inbuf_size;
   outbuf_size = avpicture_get_size(STREAM_PIX_FMT, video_context->width, video_context->height);
   outbuf = av_malloc(outbuf_size);
   inbuf_size = avpicture_get_size(PIX_FMT_YUYV422, video_context->width, video_context->height);
   inbuf = av_malloc(inbuf_size);
   
   AVFrame *srcFrame, *dstFrame; 
   // Allocate space for the frames
   srcFrame = avcodec_alloc_frame(); 
   dstFrame = avcodec_alloc_frame(); 

   // Create AVFrame for YUV420 frame
   avpicture_fill((AVPicture *)dstFrame, outbuf, STREAM_PIX_FMT, video_context->width, video_context->height);

   // Create AVFrame for YUYV422 frame
   x = avpicture_fill((AVPicture *)srcFrame, buffers[0].start, PIX_FMT_YUYV422, video_context->width, video_context->height);

   // Make the conversion context
   img_convert_ctx = sws_getContext(video_context->width, video_context->height, 
                    PIX_FMT_YUYV422, 
                    video_context->width, video_context->height, STREAM_PIX_FMT, SWS_BICUBIC, 
                    NULL, NULL, NULL);
   
   sws_scale(img_convert_ctx, srcFrame->data, 
         srcFrame->linesize, 0, 
         video_context->height, 
         dstFrame->data, dstFrame->linesize);

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
   av_free(outbuf);
   av_free(inbuf);
}

void video_frame_write()
{

}

//Closes the camera and frees all memory
void video_close(){
   avcodec_close(video_context);
   av_free(video_context);
	int closed = close(camera_fd);
	if(closed == 0)
		camera_fd = -1;
}

   // frames parameters
   video_context->gop_size = 10; // emit one intra frame every ten frames

   video_context->pix_fmt = ;


   
void add_video_stream(enum CodecID codec_id)
{
	video_st = av_new_stream(output_context, 0);
	if (!video_st) {
		fprintf(stderr, "Could not alloc stream\n");
		exit(1);
	}

	video_context = video_st->codec;
	video_context->codec_id = codec_id;
	video_context->codec_type = AVMEDIA_TYPE_VIDEO;

	// put sample parameters
	video_context->bit_rate = 400000;
	video_context->width = VIDEO_WIDTH;
	video_context->height = VIDEO_HEIGHT;
	// timing
	video_context->time_base = (AVRational){1, STREAM_FRAME_RATE};
	// frame type limits
	video_context->gop_size = 12; /* emit one intra frame every twelve frames at most */
	video_context->max_b_frames=2;
	// pix format
	video_context->pix_fmt = STREAM_PIX_FMT;
	// h264 parameters
   video_context->me_range = 16;
   video_context->max_qdiff = 4;
   video_context->qmin = 10;
   video_context->qmax = 51;
   video_context->qcompress = 0.6;
	// some formats want stream headers to be separate
	if(output_context->oformat->flags & AVFMT_GLOBALHEADER)
		video_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

void open_video()
{
    /* find the video encoder */
    video_codec = avcodec_find_encoder(c->codec_id);
    if (!video_codec) {
        fprintf(stderr, "video_codec not found\n");
        exit(1);
    }

    /* open the codec */
    if (avcodec_open(c, video_codec) < 0) {
        fprintf(stderr, "could not open video_codec\n");
        exit(1);
    }

    video_outbuf = NULL;
    if (!(output_context->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        /* buffers passed into lav* can be allocated any way you prefer,
           as long as they're aligned enough for the architecture, and
           they're freed appropriately (such as using av_free for buffers
           allocated with av_malloc) */
        video_outbuf_size = 200000;
        video_outbuf = av_malloc(video_outbuf_size);
    }

    /* allocate the encoded raw picture */
    picture = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!picture) {
        fprintf(stderr, "Could not allocate picture\n");
        exit(1);
    }

    /* if the output format is not YUV420P, then a temporary YUV420P
       picture is needed too. It is then converted to the required
       output format */
    tmp_picture = NULL;
    if (c->pix_fmt != PIX_FMT_YUV420P) {
        tmp_picture = alloc_picture(PIX_FMT_YUV420P, c->width, c->height);
        if (!tmp_picture) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
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
/*
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

}*/

void set_format(){
	// Set the format of the image from the video device
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = VIDEO_WIDTH;
	format.fmt.pix.height = VIDEO_HEIGHT;
	format.fmt.pix.field = V4L2_FIELD_INTERLACED;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	if(ioctl(camera_fd, VIDIOC_S_FMT, &format) == -1){
		perror("VIDIOC_S_FMT");
		exit( EXIT_FAILURE );
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
