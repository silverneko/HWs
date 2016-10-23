CXX = nvcc
CXXFLAGS = -std=c++11 -O2
CXXFLAGS += -arch sm_30

targets = result.y4m gen
objs = lab2.o main.o

all: $(targets)

lab2.o: lab2.cu lab2.h
	$(CXX) $(CXXFLAGS) -c lab2.cu -o lab2.o

main.o: main.cu lab2.h SyncedMemory.h
	$(CXX) $(CXXFLAGS) -c main.cu -o main.o

result.y4m: gen
	./gen

gen: $(objs)
	$(CXX) $(CXXFLAGS) $(objs) -o gen

output.mkv: result.y4m
	avconv -i result.y4m output.mkv

output.mp4: result.y4m
	avconv -i result.y4m output.mp4

clean:
	rm -rf $(targets) $(objs) output.mkv
