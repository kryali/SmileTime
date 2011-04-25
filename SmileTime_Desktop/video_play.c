#include "video_play.h"

// Super helpful link: http://sdl.beuc.net/sdl.wiki/SDL_Overlay

//The surfaces that will be used
SDL_Surface *message = NULL;
SDL_Surface *background = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *cam_surface = NULL;

SDL_Overlay * overlay_camera = NULL;
SDL_Overlay * overlay_phone = NULL;

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
	SDL_Surface *icon;
	icon = IMG_Load("icon.png");
	SDL_WM_SetIcon(icon, NULL);

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
	if(buffers == NULL){
		exit(EXIT_FAILURE);
	}

	overlay_camera->pixels[0] = decompressed_frame_camera;
	SDL_UnlockYUVOverlay(overlay_camera);

	if((decompressed_frame_phone) != NULL){
		SDL_LockYUVOverlay(overlay_phone);
		overlay_phone->pixels[0] = decompressed_frame_phone;
		SDL_UnlockYUVOverlay(overlay_phone);
	}

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


// This function listens on a port and sets up accepting a connection
void init_av_socket(){
	int listenfd = listen_on_port(VIDEO_PORT);
}

void init_udp_av(){
	int s, i, slen=sizeof(si_me);

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		perror("socket");

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(VIDEO_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, &si_me, sizeof(si_me))==-1)
		perror("bind");
	printf("[VIDEO] UDP Socket is bound\n");
}


void * read_jpg(int fd){

    char * buf = malloc(10);
    printf("Waiting for data...\n");
    int readSize;
    int writeSize;
    int packetSize = 0;
	int jpgSize = 0;
    if( (readSize = read(fd, &packetSize, sizeof(int))) == -1){
        perror("read");
        exit(0);
    }
    printf("Size of the image file: %d\n", packetSize);
	void * jpg_buffer = malloc(packetSize);
	memset(jpg_buffer, 0, packetSize);
    while(jpgSize < (packetSize-4)){
        memset(buf, 0, 10);
        if( (readSize = read(fd, jpg_buffer, packetSize)) == -1){
            perror("read");
        }
		printf("Read %d bytes\n", readSize);
        if(readSize == 0){
			break;
        }
		jpgSize += readSize;
    }
	printf("File of %db written\n", jpgSize);
	free(buf);
	return jpg_buffer;

}

int width1 = VIDEO_WIDTH;
int height1 = VIDEO_HEIGHT;
void video_frame_decompress()
{
/*
	FILE* jpgfile = fopen("kiran.jpg", "r");
	fseek(jpgfile, 0, SEEK_END);
	int fileSize = ftell(jpgfile);
	void* buffe = malloc(fileSize);
	rewind(jpgfile);
	fread(buffe, fileSize, 1, jpgfile);
*/
//	void * buffe = read_jpg(fd);
//	jpeg_decode(&decompressed_frame_phone, buffe, &width1, &height1);
//	fclose(jpgfile);
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
	//printf("pan:%d\n", pan);
	xioctl(V4L2_CID_PAN_RELATIVE, pan);
}

void tilt_relative(int tilt){
	//printf("tilt:%d\n", tilt);
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
