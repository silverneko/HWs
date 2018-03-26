CC:=gcc
CXX:=g++

CFLAGS:=-O2 -std=c++11 -Wall -Wextra

all: warm_up problem1 problem2
	./warm_up
	./problem1
	./problem2

warm_up: warm_up.cpp
	$(CXX) $(CFLAGS) warm_up.cpp -o warm_up

problem1: problem1.cpp
	$(CXX) $(CFLAGS) problem1.cpp -o problem1

problem2: problem2.cpp
	$(CXX) $(CFLAGS) problem2.cpp -o problem2
