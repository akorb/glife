CXX=g++
CXXFLAGS=-Wall -pthread -g -O3

release:
	$(CXX) $(CXXFLAGS) -o glife glife.cpp

benchmark:
	$(CXX) $(CXXFLAGS) -D BENCHMARK -o glife glife.cpp

clean:
	rm glife
