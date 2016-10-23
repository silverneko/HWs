CC=clang++
CFLAG=-std=c++11 -O2 -Wall
target=system
header=binomial_heap.h

all: $(target)

system: $(header) main.cpp
	$(CC) $(CFLAG) main.cpp -o system

clean:
	rm -rf $(target)
