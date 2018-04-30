CC:=gcc
CXX:=g++

CFLAGS:=-O2 -std=c++11 -Wall -Wextra

all: problem1 problem2
	./problem1a raw/sample1.raw B.raw
	./problem1b raw/sample1.raw C.raw
	./problem1c raw/sample1.raw S.raw
	./problem2a raw/sample2.raw K.raw
	./problem2b raw/sample2.raw K.raw E.raw

problem1: problem1a.cpp problem1b.cpp problem1c.cpp
	$(CXX) $(CFLAGS) problem1a.cpp -o problem1a
	$(CXX) $(CFLAGS) problem1b.cpp -o problem1b
	$(CXX) $(CFLAGS) problem1c.cpp -o problem1c

problem2: problem2a.cpp problem2b.cpp
	$(CXX) $(CFLAGS) problem2a.cpp -o problem2a
	$(CXX) $(CFLAGS) problem2b.cpp -o problem2b
