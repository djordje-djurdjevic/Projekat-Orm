CC = gcc
CFLAGS = -Wall -Wextra

all: tracker peer #compiles both

tracker: tracker.c
	$(CC) $(CFLAGS) segment.c tracker.c -o tracker

peer: peer.c
	$(CC) $(CFLAGS) peer.c -o peer

clean:
	rm -f tracker peer
	rm -f segments/* peers/*