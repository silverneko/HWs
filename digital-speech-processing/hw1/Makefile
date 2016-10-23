.PHONY: all clean

CFLAGS+=
LDFLAGS+=-lm     # link to math library

TARGET=train test

all: $(TARGET)
# type make/make all to compile test_hmm

train: train.cpp hmm.h
	g++ train.cpp -o train -O2 -std=c++11 -Wall -Wextra

test: test.cpp hmm.h
	g++ test.cpp -o test -O2 -std=c++11 -Wall -Wextra

clean:
	$(RM) $(TARGET)   # type make clean to remove the compiled file
