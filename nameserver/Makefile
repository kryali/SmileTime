INSTALLROOT=$(PWD)

CC=gcc -pthread 

MATH_LIB=-lm 

CFLAGS = -O0  -Wall -g -L/nmnt/work1/cs414/G6/x264/lib/ -I/nmnt/work1/cs414/G6/x264/include/

NAMESERVER_OBJ= nameserver.o

all:	nameserver

clean:
	@echo "Cleaning up directory."
	rm -f *.o nameserver

# Applications:

nameserver:	$(NAMESERVER_OBJ)
	$(CC)   $(NAMESERVER_OBJ) -o $@


nameserver.o: nameserver.c nameserver.h
