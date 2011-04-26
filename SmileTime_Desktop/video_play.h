#include <stdio.h>
#include <jpeglib.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "video_record.h"

#ifndef VIDEO_PLAY_H
#define VIDEO_PLAY_H

#define SDL_YV12_OVERLAY  0x32315659  /* Planar mode: Y + V + U */
#define SDL_IYUV_OVERLAY  0x56555949  /* Planar mode: Y + U + V */
#define SDL_YUY2_OVERLAY  0x32595559  /* Packed mode: Y0+U0+Y1+V0 */
#define SDL_UYVY_OVERLAY  0x59565955  /* Packed mode: U0+Y0+V0+Y1 */

#define UDP_MAX 15000

	SDL_Event event;
	int video_socket;
	int audio_socket;
	struct sockaddr_in si_me; // UDP Sock_addr used for recv data
	struct sockaddr_in si_audio; // UDP Sock_addr used for recv data
	void * jpgBuffer;
	void * audioBuffer;

	void print_overlay_info();
	void init_udp_audio();
	int accept_connection_s(int socket, int protocol);
	void video_play_init();
	void video_frame_display();
	void video_frame_decompress();
	void* read_audio_packet();
	void * read_jpg(int fd);
	int sdl_init();
	void sdl_quit();
	void keyboard_capture();
	void xioctl(int ctrl, int value);
	void init_av_socket();
	void pan_relative(int pan);
	void tilt_relative(int tilt);
	void pan_reset();
	void tilt_reset();
	void panTilt_reset();
#endif
