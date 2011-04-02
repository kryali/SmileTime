INSTALLROOT=$(PWD)

CC=gcc -pthread

FFMPEG_LIB = -lavformat -lavcodec -lavutil -lm -lswscale -lz -lbz2 -lx264 -lmp3lame

SDLLIBS = $(shell sdl-config --libs) 
SDLFLAGS = $(shell sdl-config --cflags)

MATH_LIB=-lm 
JPEG_LIB=-ljpeg

CFLAGS = -O0 $(SDLFLAGS) -Wall -g -L/nmnt/work1/cs414/G6/x264/lib/ -I/nmnt/work1/cs414/G6/x264/include/

REC_OBJECTS= recorder.o audio_record.o video_record.o video_play.o io_tools.o recorder_server.o recorder_client.o network_packet.o helper.o
PLAY_OBJECTS= player.o audio_play.o video_play.o io_tools.o player_client.o network_packet.o helper.o
NAMESERVER_OBJ= nameserver.o helper.o

all:	recorder player nameserver

clean:
	@echo "Cleaning up directory."
	rm -f *.o recorder player

# Applications:
recorder:	$(REC_OBJECTS)
	$(CC)	$(CFLAGS) $(REC_OBJECTS) $(MATH_LIB) $(JPEG_LIB) $(SDLLIBS) $(FFMPEG_LIB) -o $@

player:	$(PLAY_OBJECTS)
	$(CC)	$(CFLAGS) $(PLAY_OBJECTS) $(MATH_LIB) $(JPEG_LIB) $(SDLLIBS) $(FFMPEG_LIB) -o $@

nameserver:	$(NAMESERVER_OBJ)
	$(CC)   $(NAMESERVER_OBJ) -o $@


player_client.o: player_client.c player_client.h

recorder_server.o: recorder_server.c recorder_server.h

recorder_client.o: recorder_client.c recorder_client.h

recorder.o:	recorder.c

player.o:	player.c

video_play.o:	video_play.c video_play.h

video_record.o:	video_record.c video_record.h

io_tools.o:	io_tools.c io_tools.h

audio_record.o:	audio_record.c audio_record.h

audio_play.o:	audio_play.c audio_play.h

nameserver.o: nameserver.c nameserver.h

network_packet.o: network_packet.c network_packet.h

helper.o: helper.c helper.h
