#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "video_record.h"
#include "video_play.h"
#include "audio_record.h"
#include "io_tools.h"

char* defaultPath = "/nmnt/work1/cs414/G6/";
int stopRecording = 0;

void usage()
{
    printf("\n\
    recorder USAGE:    recorder FILE_NAME\n\
    Capture the video and audio data and save to the FILE_NAME. Press CTRL+C to exit\n\n");
}

void onExit()
{
    printf("[MAIN] CTRL+C has been received. Add logic here before the program exits\n");
    video_close();
    stopRecording = 1;
}

int main(int argc, char*argv[])
{
    if (argc != 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        usage();
        return 0;
    }
    signal( SIGINT,&onExit);

    printf("[MAIN] I am going to record both video and audio data to the file: %s\n", argv[1]);

    video_record_init();
    video_play_init();
    audio_record_init();
/*
    while(stopRecording == 0)
    {
        video_frame_copy();
        video_frame_display();
        video_frame_compress();
        audio_segment_copy();
        audio_segment_compress();

        video_save();
        audio_save();

        printf("[MAIN] One frame has been captured, sleep for a while and continue...\n");
        usleep(1000000);
    }
*/

    printf("[MAIN] Quit recorder\n");
    return 0;
}
