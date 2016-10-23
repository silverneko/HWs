CC = clang++ 
CFLAGS = -O2 -std=c++11 -Wall
target = 3_4_1 3_4_2

all: $(target)

3_4_1: 3_4_1.cpp
	$(CC) $(CFLAGS) 3_4_1.cpp -o 3_4_1.out

3_4_2: 3_4_1.cpp
	$(CC) $(CFLAGS) -DTASK_3_4_2 3_4_1.cpp -o 3_4_2.out

run1: 3_4_1
	./3_4_1.out < ./input > ./output

run2: 3_4_2
	./3_4_2.out < ./input > ./output

clean:
	rm -f 3_4_1.out 3_4_2.out
