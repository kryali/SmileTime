#ifndef RECORDER_SERVER_H
#define RECORDER_SERVER_H

#include "include.h"
#include "video_play.h"
#include "video_record.h"
#include "audio_record.h"

int recorder_control_socket;
int recorder_audio_socket;
int recorder_video_socket;
int controlfd;
int audiofd;
int videofd;
int av_protocol;

void send_init_control_packet( AVStream* steam0, AVStream* stream1 );
int init_connection(int port, int protocol);
int accept_connection(int socket, int protocol);
void establish_video_connection();
void establish_audio_connection();
void establish_control_connection();
void establish_peer_connections(int protocol);
void listen_control_packets();
void stream_video_packets();
void stream_audio_packets();
void register_nameserver(char * name, char * protocol, char * control_port);
char * nameServerMsg(char * name, char * ip, char * port, char * protocol, int * size);
char * getIP();

#endif
