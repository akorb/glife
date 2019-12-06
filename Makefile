CC=gcc
CFLAGS=-Wall -lstdc++

all:
	$(CC) $(CFLAGS) -g -o glife glife.cpp

clean:
	rm glife