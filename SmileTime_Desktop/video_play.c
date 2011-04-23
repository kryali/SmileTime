#include "video_play.h"

#define SDL_YV12_OVERLAY  0x32315659  /* Planar mode: Y + V + U */
#define SDL_IYUV_OVERLAY  0x56555949  /* Planar mode: Y + U + V */
#define SDL_YUY2_OVERLAY  0x32595559  /* Packed mode: Y0+U0+Y1+V0 */
#define SDL_UYVY_OVERLAY  0x59565955  /* Packed mode: U0+Y0+V0+Y1 */

// Super helpful link: http://sdl.beuc.net/sdl.wiki/SDL_Overlay

//The surfaces that will be used
SDL_Surface *message = NULL;
SDL_Surface *background = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *cam_surface = NULL;

SDL_Overlay * overlay = NULL;

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

	//Set up the screen
	screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE | SDL_ANYFORMAT );

	//If there was in error in setting up the screen
	if( screen == NULL )
	{
		printf("Screen setup failed!\n");
		return 1;    
	}

	//Set the window caption
	SDL_WM_SetCaption( "smiletime!", NULL );


	overlay = SDL_CreateYUVOverlay(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_YUY2_OVERLAY, screen);
	if(overlay == NULL){
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
	printf("Planes :%d\n", overlay->planes);
	printf("WIDTH: %d HEIGHT: %d\n", overlay->w, overlay->h);
	printf("HARDWARE ACCELERATION: %d\n", overlay->hw_overlay);
	printf("Format: 0x%x\n", overlay->format);
}

void video_frame_display(int bufferIndex)
{
  //printf("[V_PLAY] This function displays the video frame on the screen\n");
	SDL_LockYUVOverlay(overlay);
	if(buffers == NULL){
		exit(EXIT_FAILURE);
	}

	overlay->pixels[0] = decompressed_frame;
	SDL_UnlockYUVOverlay(overlay);

	SDL_Rect video_rect;
	video_rect.x = 0;
	video_rect.y = 0;
	video_rect.w = SCREEN_WIDTH;
	video_rect.h = SCREEN_HEIGHT;
	SDL_DisplayYUVOverlay(overlay, &video_rect);

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
						pan_relative(-250);
					break;
	      		case SDLK_RIGHT:
						pan_relative(250);
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
