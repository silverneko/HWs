CC:=gcc
CXX:=g++

CFLAGS:=-O2 -std=c++11 -Wall -Wextra

all: problem1 problem2
	./problem1a raw/sample1.raw problem1a.out
	./problem1b raw/sample2.raw problem1b.out
	./problem2a raw/sample3.raw problem2a.out
	./problem2b problem2a.out.raw problem2b.out

problem1: problem1a.cpp problem1b.cpp
	$(CXX) $(CFLAGS) problem1a.cpp -o problem1a
	$(CXX) $(CFLAGS) problem1b.cpp -o problem1b

problem2: problem2a.cpp problem2b.cpp
	$(CXX) $(CFLAGS) problem2a.cpp -o problem2a
	$(CXX) $(CFLAGS) problem2b.cpp -o problem2b

