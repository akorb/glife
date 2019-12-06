CC=gcc
CFLAGS=-Wall -lstdc++

all:
	$(CC) $(CFLAGS) -o glife glife.cpp

clean:
	rm glife