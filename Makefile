CC=g++
CFLAGS=-Wall -pthread

all:
	$(CC) $(CFLAGS) -O3 -o glife glife.cpp

debug:
	$(CC) $(CFLAGS) -g -o glife glife.cpp

clean:
	rm glife
