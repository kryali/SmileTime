#include "audio_record.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>


int microphone_fd = -1;
char* microphone_name = "/dev/video";

void audio_record_init()
{
    	//open microphone
	microphone_fd = open(microphone_name, O_RDWR);
	if(microphone_fd == -1){
		printf("error opening microphone %s\n", microphone_name);
		return;
	}
	printf("File Descriptor: %d\n", microphone_fd);

	struct v4l2_audio audio;

	memset (&audio, 0, sizeof (audio));

	if (-1 == ioctl (microphone_fd, VIDIOC_G_AUDIO, &audio)) {
	        perror ("VIDIOC_G_AUDIO");
 	       exit (EXIT_FAILURE);
	}

	printf ("Current input: %s\n", audio.name);

}

void audio_segment_copy()
{
    printf("[A_REC] This function copies the audio segment from sound driver\n");
}

void audio_segment_compress()
{
    printf("[A_REC] This function should compress the raw wav to MP3 or AAC\n");
}
