CXX=g++
CXXFLAGS=-Wall -pthread -g -O3

all:
	$(CXX) $(CXXFLAGS) -o glife glife.cpp

clean:
	rm glife
