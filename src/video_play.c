#include <stdio.h>
#include "SDL/SDL.h"

//The surfaces that will be used
SDL_Surface *message = NULL;
SDL_Surface *background = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *cam_surface = NULL;

//The attributes of the screen
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;
const int CAM_WIDTH = 320;
const int CAM_HEIGHT = 240;

int sdl_init(){
	SDL_Event event;
	//Initialize all SDL subsystems
	if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
	{
		return 1;    
	}

	//Set up the screen
	screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

	//If there was in error in setting up the screen
	if( screen == NULL )
	{
		return 1;    
	}

	//Set the window caption
	SDL_WM_SetCaption( "Video4Linux + SDL", NULL );

	//Load the images

	//createCamImage (CAM_WIDTH*2,CAM_HEIGHT*2);

	//apply_surface (0,0,cam_surface,screen);
	//Update the screen
	if( SDL_Flip( screen ) == -1 )
	{
		return 1;    
	}

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

void video_frame_display()
{
    printf("[V_PLAY] This function displays the video frame on the screen\n");
}

