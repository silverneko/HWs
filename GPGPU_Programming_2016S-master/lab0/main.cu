#include <cstdio>
#include <cstdlib>
#include "SyncedMemory.h"

#define CHECK {\
	auto e = cudaDeviceSynchronize();\
	if (e != cudaSuccess) {\
		printf("At " __FILE__ ":%d, %s\n", __LINE__, cudaGetErrorString(e));\
		abort();\
	}\
}

__global__ void SomeTransform(char *input_gpu, int fsize) {
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < fsize and input_gpu[idx] != '\n') {
		input_gpu[idx] = '!';
	}
}

__global__ void myTransform(char *, int);

int main(int argc, char **argv)
{
	// init, and check
	if (argc != 2) {
		printf("Usage %s <input text file>\n", argv[0]);
		abort();
	}
	FILE *fp = fopen(argv[1], "r");
	if (not fp) {
		printf("Cannot open %s", argv[1]);
		abort();
	}
	// get file size
	fseek(fp, 0, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// read files
	MemoryBuffer<char> text(fsize+1);
	auto text_smem = text.CreateSync(fsize);
	CHECK;
	fread(text_smem.get_cpu_wo(), 1, fsize, fp);
	text_smem.get_cpu_wo()[fsize] = '\0';
	fclose(fp);

	// TODO: do your transform here
	char *input_gpu = text_smem.get_gpu_rw();
	// An example: transform the first 64 characters to '!'
	// Don't transform over the tail
	// And don't transform the line breaks
        // SomeTransform<<<2, 32>>>(input_gpu, fsize);

        // set `gridDim` and `blockDim`
        int blockCount = 2;
        int threadCount = 512;
        // launch kernel
        // myTransform() changes lower case letters to upper case
        // and shift letters by the distance of 10
        // for example: 'a' -> 'K', 'b' -> 'L', 'z' -> 'J'
        myTransform<<<blockCount, threadCount>>>(input_gpu, fsize);

	puts(text_smem.get_cpu_ro());
	return 0;
}

__global__ void myTransform(char * input, int len) {
  // calculate the ID of this thread and use it as the index of the input string
  int globalID = blockIdx.x * blockDim.x + threadIdx.x;
  // grid-stride loop
  for(; globalID < len; globalID += blockDim.x * gridDim.x){
    if('a' <= input[globalID] && input[globalID] <= 'z'){
      input[globalID] += 'A' - 'a';
    }
    if('A' <= input[globalID] && input[globalID] <= 'Z'){
      input[globalID] = (((int)input[globalID] - 'A' + 10) % 26) + 'A';
    }
  }
}

