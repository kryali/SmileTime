#ifndef SMILETIME_H
#define SMILETIME_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "video_record.h"
#include "video_play.h"
#include "audio_record.h"
#include "audio_play.h"
#include "io_tools.h"

#include "recorder_server.h"
#include "recorder_client.h"

#include "include.h"
#include "chatwindow.h"

pthread_t control_network_thread_id;
pthread_t video_capture_thread_id;
pthread_t audio_capture_thread_id;
pthread_t keyboard_thread_id;
pthread_t chat_thread_id;
pthread_t audio_decode_thread_id;
pthread_t video_decode_thread_id;
pthread_t stats_thread_id;
pthread_t latency_thread_id;

char* peer_port;
char* protocol;

void usage();
void onExit();
void * startVideoEncoding();
void * startAudioEncoding();
void * startVideoDecoding();
void * startAudioDecoding();
void * captureKeyboard();
void connect_to_nameserver(int argc, char*argv[]);
#endif
