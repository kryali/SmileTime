#include <stdio.h>
#include "SDL/SDL.h"
#include "video_record.h"

#ifndef VIDEO_PLAY_H
#define VIDEO_PLAY_H
	void print_overlay_info();
	void video_play_init();
	void video_frame_decompress();
	void video_frame_display(int bufferIndex);
	int sdl_init();
	void sdl_quit();
	void keyboard_capture();
#endif
