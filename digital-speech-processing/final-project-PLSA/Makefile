CXXFLAGS=-std=c++11 -O2 -Wall -Wextra
targets = parse train test

all: $(targets)

parse: parse.cpp

train: train.cpp Utils.hpp

test: test.cpp Utils.hpp

clean:
	@rm $(targets)
