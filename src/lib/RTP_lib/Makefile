BIN		= Rtp_Exemple_Send
OBJECTS 	= Rtp_Exemple_Send.o RTP.o Hdr_Builder.o Network.o Utils.o Rtp_Exemple_Receive_Print.o
BIN2		= Rtp_Exemple_Receive
OBJECTS2	= Rtp_Exemple_Receive.o RTP.o Hdr_Builder.o Network.o Utils.o Rtp_Exemple_Receive_Print.o
CC = gcc

all: send receive

send: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BIN)

Rtp_Exemple_Send.o: Rtp_Exemple_Send.c
	$(CC) -c Rtp_Exemple_Send.c

RTP.o: RTP.c
	$(CC) -c RTP.c

Hdr_Builder.o: Hdr_Builder.c
	$(CC) -c Hdr_Builder.c

Network.o: Network.c
	$(CC) -c Network.c

Utils.o: Utils.c
	$(CC) -c Utils.c

receive: $(OBJECTS2)
	$(CC) $(OBJECTS2) -o $(BIN2)

Rtp_Exemple_Receive.o: Rtp_Exemple_Receive.c
	$(CC) -c Rtp_Exemple_Receive.c

Rtp_Exemple_Receive_Print.o: Rtp_Exemple_Receive_Print.c
	$(CC) -c Rtp_Exemple_Receive_Print.c

clean:
	rm -f $(OBJECTS) $(OBJECTS2) $(BIN) $(BIN2) *~ *.core
