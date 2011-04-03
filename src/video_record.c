#include "video_record.h"
#include "recorder_server.h"
#include "include.h"

//SOURCES:
/*
http://v4l2spec.bytesex.org/spec/book1.htm
http://v4l2spec.bytesex.org/spec/capture-example.html
*/
char* camera_name = "/dev/video0";
int enc_size;

AVFormatContext *output_context;
AVPacket video_pkt;

AVFrame *yuv420_frame, *yuyv422_frame;
uint8_t *video_outbuf;
int frame_count, video_outbuf_size;

//This function initialize the camera device and V4L2 interface
void video_record_init(AVOutputFormat *fmt, AVFormatContext *oc){
	video_st = NULL;
	output_context = oc;
	
	if (fmt->video_codec != CODEC_ID_NONE) {
		add_video_stream(fmt->video_codec);
	}
	if (video_st) {
		open_video();
	}
	
	//open camera
	camera_fd = open(camera_name, O_RDWR );
	if(camera_fd == -1){
		printf("error opening camera %s\n", camera_name);
		return;
	}
	buffers = NULL;
	print_Camera_Info();
	set_format();	
	mmap_init();	
  //printf("[V_REC] This function initialize the camera device and V4L2 interface\n");
}

//This function copies the raw image from webcam frame buffer to program memory through V4L2 interface
int video_frame_copy(){

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
	return bufQ.index;
}

// This function should compress the raw image to JPEG image, or MPEG-4 or H.264 frame if you choose to implemente that feature
void video_frame_compress( int bufferIndex){
   static struct SwsContext *img_convert_ctx;
	yuyv422_frame = alloc_frame(buffers[bufferIndex].start, CAMERA_PIX_FMT, video_context->width, video_context->height);
   // Make the conversion context
   img_convert_ctx = sws_getContext(
   	video_context->width, video_context->height, CAMERA_PIX_FMT, 
		video_context->width, video_context->height, STREAM_PIX_FMT,
		SWS_BICUBIC, NULL, NULL, NULL);
   
   sws_scale(img_convert_ctx, 
   	yuyv422_frame->data, yuyv422_frame->linesize, 
		0, video_context->height, 
		yuv420_frame->data, yuv420_frame->linesize);

	// Encode the frame
	enc_size = avcodec_encode_video(video_context, video_outbuf, video_outbuf_size, yuv420_frame);
	if (enc_size > 0) {
		av_init_packet(&video_pkt);
		if (video_context->coded_frame->pts != AV_NOPTS_VALUE){
					//printf("codedframe: %d\n",video_context->coded_frame->pts);
					//printf("contexttimebaseden: %d\n",video_context->time_base.den);
					//printf("videosttimebaseden: %d\n",video_st->time_base.den);
			//video_pkt.pts= av_rescale_q(video_context->coded_frame->pts, video_context->time_base, video_st->time_base);
					//printf("pts`	: %d\n",video_pkt.pts);
					video_pkt.pts= av_rescale(video_context->coded_frame->pts, TIME_TOTAL, FRAMES_ENCODED);
		}
		if(video_context->coded_frame->key_frame)
			video_pkt.flags |= AV_PKT_FLAG_KEY;
		video_pkt.stream_index= video_st->index;
 		video_pkt.data= video_outbuf;
		video_pkt.size= enc_size;
	}
	av_free(yuyv422_frame);
}

void video_frame_write()
{
  // Transmit the video packet
	av_packet av;
	av.av_data = video_pkt;
	HTTP_packet* http = av_to_network_packet(&av);
	int size = http->length-1;
	if( write(videofd, &size, sizeof(size)) ==0){
		perror("write");
	}
	xwrite(videofd, http);
	destroy_HTTP_packet(http);
}

//Closes the camera and frees all memory
void video_close(){

	av_free(yuv420_frame->data[0]);
	av_free(yuv420_frame);
	av_free(video_outbuf);
    avcodec_close(video_context);
    av_free(video_context);
	int closed = close(camera_fd);
	if(closed == 0)
		camera_fd = -1;
}
   
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
//	video_context->time_base.den = STREAM_FRAME_RATE;
//	video_context->time_base.den = 1;
	// frame type limits
	video_context->gop_size = 12; // emit one intra frame every twelve frames at most
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

AVFrame *alloc_frame(uint8_t *frame_buf, enum PixelFormat pix_fmt, int width, int height)
{
	AVFrame *frame;
	int size;

	frame = avcodec_alloc_frame();
	if (!frame)
		return NULL;
	if(frame_buf == NULL) {
		size = avpicture_get_size(pix_fmt, width, height);
		frame_buf = av_malloc(size);
	}
	if (!frame_buf) {
		av_free(frame);
		return NULL;
	}
	avpicture_fill((AVPicture *)frame, frame_buf, pix_fmt, width, height);
	return frame;
}

void open_video()
{
    /* find the video encoder */
    video_codec = avcodec_find_encoder(video_context->codec_id);
    if (!video_codec) {
        fprintf(stderr, "video codec not found\n");
        exit(1);
    }

    /* open the codec */
    if (avcodec_open(video_context, video_codec) < 0) {
        fprintf(stderr, "could not open video codec\n");
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
    yuv420_frame = alloc_frame(NULL, video_context->pix_fmt, video_context->width, video_context->height);
    if (!yuv420_frame) {
        fprintf(stderr, "Could not allocate yuv420_frame\n");
        exit(1);
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
