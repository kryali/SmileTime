#include "video_play.h"

#define SDL_YV12_OVERLAY  0x32315659  /* Planar mode: Y + V + U */
#define SDL_IYUV_OVERLAY  0x56555949  /* Planar mode: Y + U + V */
#define SDL_YUY2_OVERLAY  0x32595559  /* Packed mode: Y0+U0+Y1+V0 */
#define SDL_UYVY_OVERLAY  0x59565955  /* Packed mode: U0+Y0+V0+Y1 */
//#define SDL_YVYU_OVERLAY  0x55595659  /* Packed mode: Y0+V0+Y1+U0 */`

// Super helpful link: http://sdl.beuc.net/sdl.wiki/SDL_Overlay`

//The surfaces that will be used
SDL_Surface *message = NULL;
SDL_Surface *background = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *cam_surface = NULL;

SDL_Overlay * overlay = NULL;

//The attributes of the screen
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 16;
const int CAM_WIDTH = 320;
const int CAM_HEIGHT = 240;
SDL_Event event;

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
	SDL_WM_SetCaption( "Logitech View", NULL );


	overlay = SDL_CreateYUVOverlay(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_YUY2_OVERLAY, screen);
	if(overlay == NULL){
		printf("Failed to create Overlay\n");
		exit(EXIT_FAILURE);
	}
	
	//Load the images

	//createCamImage (CAM_WIDTH*2,CAM_HEIGHT*2);

	//apply_surface (0,0,cam_surface,screen);
	//Update the screen
/*
	if( SDL_Flip( screen ) == -1 )
	{
		return 1;    
	}
*/
//	SDL_Delay(1100);
  return 0;

}

void video_play_init()
{
    printf("[V_PLAY] This function initiates the libraries used to display video on screen\n");
    sdl_init();
}

void video_frame_decompress()
{
    printf("[V_PLAY] This function decompresses the video frame read from media file\n");
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
//	if(overlay->pixels[0] == NULL)
	overlay->pixels[0] = malloc(buffers[bufferIndex].length);
	memcpy(overlay->pixels[0], buffers[bufferIndex].start, buffers[bufferIndex].length);
//	overlay->pixels[bufferIndex] = buffers[bufferIndex].start;

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

void keyboard_capture()
{
	while (SDL_PollEvent(&event))   //Poll our SDL key event for any keystrokes.
	{

	switch(event.type) {
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
				case SDLK_LEFT:
					printf("left\n");
					pan_relative(-50);
				break;
      		case SDLK_RIGHT:
					printf("right\n");
					pan_relative(50);
				break;
      		case SDLK_UP:
					printf("up\n");
					tilt_relative(25);
				break;
      		case SDLK_DOWN:
					printf("down\n");
					tilt_relative(-25);
				break;
				default:
				break;

				}
		}
	}
}
