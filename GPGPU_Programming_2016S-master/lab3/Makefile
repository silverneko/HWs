NVCC = nvcc
NVCCFLAGS = -std=c++11 -O2 -arch sm_30
RM = rm -rf

TARGETS = a.out
OBJS = main.o lab3.o pgm.o
INCLUDES = lab3.h SyncedMemory.h pgm.h

ifdef DEBUG
NVCCFLAGS += -DDEBUG
endif

.PHONY: all clean run

$(TARGETS): $(OBJS)
	$(NVCC) $(NVCCFLAGS) $(OBJS) -o $@

lab3.o: $(INCLUDES) lab3.cu
	$(NVCC) $(NVCCFLAGS) -c lab3.cu -o $@

main.o: $(INCLUDES) main.cu
	$(NVCC) $(NVCCFLAGS) -c main.cu -o $@

pgm.o: $(INCLUDES) pgm.cpp
	$(NVCC) $(NVCCFLAGS) -c pgm.cpp -o $@

all: $(TARGETS)

run: $(TARGETS)
	./$(TARGETS) img_background.ppm img_target.ppm img_mask.pgm 130 600 output.ppm

clean:
	$(RM) $(OBJS) $(TARGETS)

