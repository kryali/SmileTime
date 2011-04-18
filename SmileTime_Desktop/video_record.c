#include "video_record.h"

//SOURCES:
/*
http://v4l2spec.bytesex.org/spec/book1.htm
http://v4l2spec.bytesex.org/spec/capture-example.html
*/
char* camera_name = "/dev/video0";
Buffer video_pkt;
struct jpeg_compress_struct cinfo;
struct jpeg_destination_mgr mgr;
JSAMPROW row_pointer[1];
JOCTET * compressed_data;

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
	mmap_init();
	
	//Set up video packet queue
	videoq = malloc(sizeof(BufferQueue));
	buffer_queue_init(videoq);
}

void video_compress_init(){
	//create jpeg compressor
	jpeg_create_compress(&cinfo);

//TEST

	char* filename = "test.jpg";
	FILE *outfile = fopen( filename, "wb" );
	
	if ( !outfile )
	{
		printf("Error opening output jpeg file %s\n!", filename );
		exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);

//TEST


	//set destination manager to an output buffer
	/*mgr.init_destination = init_destination;
	mgr.empty_output_buffer = empty_output_buffer;
	mgr.term_destination = term_destination;
	cinfo.dest = &mgr;*/

	//set image info
	cinfo.image_width = VIDEO_WIDTH; 	/* image width and height, in pixels */
	cinfo.image_height = VIDEO_HEIGHT;
	cinfo.input_components = 3;	/* # of color components per pixel */
	cinfo.in_color_space = JCS_YCbCr; /* colorspace of input image */
	jpeg_set_defaults(&cinfo);
}

//This function copies the raw image from webcam frame buffer to program memory through V4L2 interface
int video_frame_copy()
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
	return bufQ.index;
}

// This function should compress the raw image to JPEG image.
void video_frame_compress( int bufferIndex)
{
	char* raw_image = buffers[bufferIndex].start;
	printf("Compression start!\n");
	jpeg_start_compress(&cinfo, FALSE);
	while( cinfo.next_scanline < cinfo.image_height )
	{
		printf("while!\n");
		row_pointer[0] = &raw_image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines( &cinfo, row_pointer, 1 );
	}
	jpeg_finish_compress(&cinfo);
	//buffers[bufferIndex];
	//video_pkt = ;
	printf("Compression success!\n");
}

void video_frame_queue()
{
	buffer_queue_put(videoq, &video_pkt);
}


Buffer net_pkt;
void video_frame_write()
{
	//if(videoq->size > 0)
		//printf("vqsize: %d\n", videoq->size);
	if(buffer_queue_get(videoq, &net_pkt) == 1)
	{
		av_packet av;
		av.buff = net_pkt;
		HTTP_packet* http = av_to_network_packet(&av);
		xwrite(videofd, http);
		destroy_HTTP_packet(http);
	}
}

//Closes the camera and frees all memory
void video_close(){
	jpeg_destroy_compress( &cinfo );
	free(videoq);
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

void init_destination(j_compress_ptr cinfo){ 
  struct jpeg_destination_mgr*dmgr = (struct jpeg_destination_mgr*)(cinfo->dest);
  compressed_data = (JOCTET*)malloc(OUTBUFFER_SIZE);
  if(!compressed_data) {
      perror("malloc");
      printf("Out of memory!\n");
      exit(1);
  }
  dmgr->next_output_byte = compressed_data;
  dmgr->free_in_buffer = OUTBUFFER_SIZE;
}

boolean empty_output_buffer(j_compress_ptr cinfo){ 
  struct jpeg_destination_mgr*dmgr = (struct jpeg_destination_mgr*)(cinfo->dest);
	//memcpy to output buffer?
  dmgr->next_output_byte = compressed_data;
  dmgr->free_in_buffer = OUTBUFFER_SIZE;
  return 1;
}

void term_destination(j_compress_ptr cinfo){ 
	struct jpeg_destination_mgr*dmgr = (struct jpeg_destination_mgr*)(cinfo->dest);
	//memcpy to output buffer?
  free(compressed_data);
  compressed_data = 0;
  dmgr->free_in_buffer = 0;
}
