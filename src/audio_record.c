#include <stdio.h>

void audio_record_init()
{
    printf("[A_REC] This function initialize the microphone device\n");
}

void audio_segment_copy()
{
    printf("[A_REC] This function copies the audio segment from sound driver\n");
}

void audio_segment_compress()
{
    printf("[A_REC] This function should compress the raw wav to MP3 or AAC\n");
}
