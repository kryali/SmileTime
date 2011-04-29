INSTALLROOT=$(PWD)

CC=gcc -pthread -lasound `pkg-config --cflags --libs gtk+-2.0`

SDLLIBS = $(shell sdl-config --libs) -lSDL_image
SDLFLAGS = $(shell sdl-config --cflags)

MATHJPG_LIB= -lm

CFLAGS = -O0 $(SDLFLAGS) -Wall -g -export-dynamic

GTKFLAGS = $(shell `pkg-config --cflags --libs gtk+-2.0`)

SMILETIME_OBJECTS=smiletime.o audio_record.o video_record.o video_play.o audio_play.o  recorder_server.o network_packet.o helper.o chatwindow.o
LUVCVIEW_OBJECTS= luvcview/utils.o luvcview/v4l2uvc.o luvcview/avilib.o luvcview/color.o

all:	smiletime

clean:
	@echo "Cleaning up directory."
	rm -f *.o smiletime

# Applications:
smiletime:	$(SMILETIME_OBJECTS)
	$(CC)	$(CFLAGS) $(SMILETIME_OBJECTS) $(LUVCVIEW_OBJECTS) $(MATH_LIB) $(SDLLIBS) -o $@

smiletime.o:	smiletime.c smiletime.h
audio_record.o:	audio_record.c audio_record.h
audio_play.o:	audio_play.c audio_play.h
video_record.o:	video_record.c video_record.h
video_play.o:	video_play.c video_play.h
recorder_server.o: recorder_server.c recorder_server.h

chatwindow.o: chatwindow.c chatwindow.h

network_packet.o: network_packet.c network_packet.h

helper.o: helper.c helper.h


