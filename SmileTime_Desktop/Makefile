INSTALLROOT=$(PWD)

CC=gcc -pthread

SDLLIBS = $(shell sdl-config --libs) 
SDLFLAGS = $(shell sdl-config --cflags)

MATHJPG_LIB=-lm

CFLAGS = -O0 $(SDLFLAGS) -Wall -g

SMILETIME_OBJECTS= smiletime.o audio_record.o video_record.o video_play.o io_tools.o recorder_server.o recorder_client.o network_packet.o helper.o buffer_queue.o
LUVCVIEW_OBJECTS= luvcview/utils.o luvcview/v4l2uvc.o luvcview/avilib.o luvcview/color.o
#PLAY_OBJECTS= player.o audio_play.o video_play.o io_tools.o player_client.o network_packet.o helper.o
NAMESERVER_OBJ= nameserver.o	

all:	smiletime

clean:
	@echo "Cleaning up directory."
	rm -f *.o smiletime

# Applications:
smiletime:	$(SMILETIME_OBJECTS)
	$(CC)	$(CFLAGS) $(SMILETIME_OBJECTS) $(LUVCVIEW_OBJECTS) $(MATH_LIB) $(SDLLIBS) -o $@

nameserver:	$(NAMESERVER_OBJ)
	$(CC)   $(NAMESERVER_OBJ) -o $@

smiletime.o:	smiletime.c
audio_record.o:	audio_record.c audio_record.h
audio_play.o:	audio_play.c audio_play.h
video_record.o:	video_record.c video_record.h
video_play.o:	video_play.c video_play.h
recorder_server.o: recorder_server.c recorder_server.h
recorder_client.o: recorder_client.c recorder_client.h

player.o:	player.c player.h
player_client.o: player_client.c player_client.h

io_tools.o:	io_tools.c io_tools.h

nameserver.o: nameserver.c nameserver.h

network_packet.o: network_packet.c network_packet.h

helper.o: helper.c helper.h

buffer_queue.o: buffer_queue.c buffer_queue.h

