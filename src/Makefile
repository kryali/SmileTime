INSTALLROOT=$(PWD)

CC=gcc -O0 -pthread

FFMPEG_LIB = -lavformat -lavcodec -lavutil -lm -lswscale -lz -lbz2 -lx264 -lvpx -lmp3lame

SDLLIBS = $(shell sdl-config --libs) 
SDLFLAGS = $(shell sdl-config --cflags)

MATH_LIB=-lm 
JPEG_LIB=-ljpeg

CFLAGS = -O2 $(SDLFLAGS) -Wall -g -L/nmnt/work1/cs414/G6/x264/lib/ -I/nmnt/work1/cs414/G6/x264/include/

REC_OBJECTS= recorder.o audio_record.o video_record.o video_play.o io_tools.o
PLAY_OBJECTS= player.o audio_play.o video_play.o io_tools.o
		
all:	recorder player

clean:
	@echo "Cleaning up directory."
	rm -f *.o recorder player

# Applications:
recorder:	$(REC_OBJECTS)
	$(CC)	$(CFLAGS) $(REC_OBJECTS) $(MATH_LIB) $(JPEG_LIB) $(SDLLIBS) $(FFMPEG_LIB) -o $@

player:	$(PLAY_OBJECTS)
	$(CC)	$(CFLAGS) $(PLAY_OBJECTS) $(MATH_LIB) $(JPEG_LIB) $(SDLLIBS) $(FFMPEG_LIB) -o $@

recorder.o:	recorder.c

player.o:	player.c

video_play.o:	video_play.c video_play.h

video_record.o:	video_record.c video_record.h

io_tools.o:	io_tools.c io_tools.h

audio_record.o:	audio_record.c audio_record.h

audio_play.o:	audio_play.c audio_play.h

