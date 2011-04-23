#include <stdio.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "video_record.h"

#ifndef VIDEO_PLAY_H
#define VIDEO_PLAY_H
	SDL_Event event;

	void print_overlay_info();
	void video_play_init();
	void video_frame_display();
	int sdl_init();
	void sdl_quit();
	void keyboard_capture();
	void xioctl(int ctrl, int value);
	void pan_relative(int pan);
	void tilt_relative(int tilt);
	void pan_reset();
	void tilt_reset();
	void panTilt_reset();
#endif
