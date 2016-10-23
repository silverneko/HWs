CXX = clang++
CXXFLAGS = -std=c++11 -Wall

all: server.cpp
	$(CXX) server.cpp -o write_server $(CXXFlAGS)
	$(CXX) server.cpp -D READ_SERVER -o read_server $(CXXFlAGS)

clean:
	rm -f read_server write_server
