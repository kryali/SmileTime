#include "video_play.h"

// Super helpful link: http://sdl.beuc.net/sdl.wiki/SDL_Overlay

//The surfaces that will be used
SDL_Surface *message = NULL;
SDL_Surface *background = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *cam_surface = NULL;
SDL_Surface *icon;

SDL_Overlay * overlay_camera = NULL;
SDL_Overlay * overlay_phone = NULL;

unsigned char * smiletime_background;

//The attributes of the screen
const int SCREEN_WIDTH = VIDEO_WIDTH;
const int SCREEN_HEIGHT = VIDEO_HEIGHT;
const int SCREEN_BPP = 24;

int sdl_init(){
	//SDL_Event event;
	//Initialize all SDL subsystems
	if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
	{
		return 1;    
	}

	//set up the icon
	icon = IMG_Load("icon.png");
	SDL_WM_SetIcon(icon, NULL);

	//set up the smiletime icon overlay
	int width1 = SCREEN_WIDTH;
	int height1 = SCREEN_HEIGHT;
	FILE* jpgfile = fopen("background.jpg", "r");
	fseek(jpgfile, 0, SEEK_END);
	int fileSize = ftell(jpgfile);
	void* buffe = malloc(fileSize);
	rewind(jpgfile);
	fread(buffe, fileSize, 1, jpgfile);
	fclose(jpgfile);
	jpeg_decode(&decompressed_frame_phone, buffe, &width1, &height1);
	free(buffe);

	//Set up the screen
	screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT*2, SCREEN_BPP, SDL_SWSURFACE | SDL_ANYFORMAT );

	//If there was in error in setting up the screen
	if( screen == NULL )
	{
		printf("Screen setup failed!\n");
		return 1;    
	}

	//Set the window caption
	SDL_WM_SetCaption( "smiletime!", NULL );


	overlay_camera = SDL_CreateYUVOverlay(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_YUY2_OVERLAY, screen);
	if(overlay_camera == NULL){
		printf("Failed to create Overlay\n");
		exit(EXIT_FAILURE);
	}

	overlay_phone = SDL_CreateYUVOverlay(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_YUY2_OVERLAY, screen);
	if(overlay_phone == NULL){
		printf("Failed to create Overlay\n");
		exit(EXIT_FAILURE);
	}
  pthread_mutex_init(&jpg_mutex, NULL);
  return 0;
}

//[V_PLAY] This function initiates the libraries used to display video on screen
void video_play_init()
{
    sdl_init();
}

void print_overlay_info(){
	/*printf("Planes :%d\n", overlay->planes);
	printf("WIDTH: %d HEIGHT: %d\n", overlay->w, overlay->h);
	printf("HARDWARE ACCELERATION: %d\n", overlay->hw_overlay);
	printf("Format: 0x%x\n", overlay->format);*/
}

void video_frame_display(int bufferIndex)
{
  //printf("[V_PLAY] This function displays the video frame on the screen\n");
	SDL_LockYUVOverlay(overlay_camera);
	SDL_LockYUVOverlay(overlay_phone);
	if(buffers == NULL){
		exit(EXIT_FAILURE);
	}

	overlay_camera->pixels[0] = decompressed_frame_camera;
	if(decompressed_frame_phone != NULL){
		overlay_phone->pixels[0] = decompressed_frame_phone;
	}

	SDL_UnlockYUVOverlay(overlay_phone);
	SDL_UnlockYUVOverlay(overlay_camera);

	SDL_Rect video_rect;
	video_rect.x = 0;
	video_rect.y = 0;
	video_rect.w = SCREEN_WIDTH;
	video_rect.h = SCREEN_HEIGHT;
	SDL_DisplayYUVOverlay(overlay_camera, &video_rect);
	video_rect.y = SCREEN_HEIGHT;
	SDL_DisplayYUVOverlay(overlay_phone, &video_rect);
}

int accept_connection_s(int socket, int protocol){
    int fd;
    if(protocol == SOCK_STREAM)
    {
        socklen_t addr_size = (socklen_t)sizeof(struct sockaddr_storage);

        struct sockaddr_storage their_addr;
        memset(&their_addr, 0, sizeof(struct sockaddr_storage));
        fd = accept( socket, (struct sockaddr *)&their_addr, &addr_size );
        if(fd == -1 ){
            perror("accept connection");
            exit(1);
        }
    }
    else if(protocol == SOCK_DGRAM)
    {
        ;//?
    }
    return fd;
}

void * read_jpg(int fd){
	struct sockaddr_in si_other;
	int jpgSize = -1;
	int sLen = sizeof(si_other);
	int readbytes = 0;
	if( (readbytes = recvfrom(fd, &jpgSize, sizeof(int), MSG_PEEK, &si_other, &sLen))== -1){
		perror("recvfrom");
	}
	pthread_mutex_lock(&bytes_received_mutex);
	bytes_received += jpgSize;
	pthread_mutex_unlock(&bytes_received_mutex);
//	printf("Received jpgSize of %d bytes from %s: [%d]\n", jpgSize,inet_ntoa(si_other.sin_addr), readbytes);
	memset(jpgBuffer, 0, UDP_MAX);
	if( (readbytes = recvfrom(fd, jpgBuffer, jpgSize+sizeof(int), 0, &si_other, &sLen))== -1){
		perror("recvfrom");
	}
//	printf("Read %d/%d of the jpg file\n", readbytes, jpgSize);
	return (jpgBuffer+sizeof(int));
}

int width1 = VIDEO_WIDTH;
int height1 = VIDEO_HEIGHT;
void video_frame_decompress()
{
	void * buffe = read_jpg(video_socket);
 
	//if(decompressed_frame_phone != NULL){
		//free(decompressed_frame_phone);
	//}
	pthread_mutex_lock(&jpg_mutex);
	jpeg_decode(&decompressed_frame_phone, buffe, &width1, &height1);
	pthread_mutex_unlock(&jpg_mutex);		
}

void sdl_quit(){
	printf("Freeing SDL\n");
//	SDL_FreeYUVOverlay(overlay);
	//SDL_Quit();
}


//http://www.zerofsck.org/2009/03/09/example-code-pan-and-tilt-your-logitech-sphere-webcam-using-python-module-lpantilt-linux-v4l2/
void xioctl(int ctrl, int value){
	struct v4l2_queryctrl qctrl;
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl (camera_fd, VIDIOC_QUERYCTRL, &qctrl)) {
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
	struct v4l2_ext_control xctrls;
	struct v4l2_ext_controls ctrls;
	xctrls.id = ctrl;
	xctrls.value = value;
	ctrls.count = 1;
	ctrls.controls = &xctrls;
	int r = 0;
	do r = ioctl (camera_fd, VIDIOC_S_EXT_CTRLS, &ctrls);
		while (-1 == r && EINTR == errno);	
}


void pan_relative(int pan){
	xioctl(V4L2_CID_PAN_RELATIVE, pan);
}

void tilt_relative(int tilt){
	xioctl(V4L2_CID_TILT_RELATIVE, tilt);
}

void pan_reset(){
	xioctl(V4L2_CID_PAN_RESET, 1);
}

void tilt_reset(){
	xioctl(V4L2_CID_TILT_RESET, 1);
}

void panTilt_reset(){
	xioctl(V4L2_CID_PANTILT_RESET, 1);
}


void keyboard_capture()
{
	while (SDL_PollEvent(&event))   //Poll our SDL key event for any keystrokes.
	{
		switch(event.type) {
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
					case SDLK_LEFT:
						pan_relative(250);
					break;

          case SDLK_RIGHT:
						pan_relative(-250);
					break;

          case SDLK_UP:
						tilt_relative(-150);
					break;

          case SDLK_DOWN:
						tilt_relative(150);
					break;

					default:
					break;
			}
		}
	}
}
