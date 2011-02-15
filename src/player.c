#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "video_play.h"
#include "audio_play.h"
#include "io_tools.h"

int frames_to_play = 5;

void usage()
{
    printf("\n\
    recorder USAGE:    player FILE_NAME\n\
    Play the video and audio data saved in the FILE_NAME. Press CTRL+C to exit\n\n");
}

void onExit()
{
    printf("[MAIN] CTRL+C has been received. Add logic here before the program exits\n");

    frames_to_play = 0;
}

int main(int argc, char*argv[])
{
    if (argc != 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        usage();
        return 0;
    }

    signal( SIGINT,&onExit);

    printf("[MAIN] I am going to play both video and audio data from file: %s\n", argv[1]);

    video_play_init();
    audio_play_init();

    while(frames_to_play > 0)
    {
        audio_read();
        video_read();

        audio_segment_decompress();
        audio_segment_playback();
        video_frame_decompress();
        printf("[MAIN] Synchronize your video frame play to the audio play.\n");
        usleep(1000000);
        //video_frame_display();

        frames_to_play --;

    }

    printf("Playback Frame Rate: *** fps\n");
    printf("[MAIN] Quit player\n");
    return 0;
}
