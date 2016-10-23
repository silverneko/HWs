#include "counting.h"
#include <cstdio>
#include <cassert>
#include <thrust/scan.h>
#include <thrust/transform.h>
#include <thrust/functional.h>
#include <thrust/device_ptr.h>
#include <thrust/execution_policy.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#include <thrust/iterator/zip_iterator.h>

__device__ __host__ int CeilDiv(int a, int b) { return (a-1)/b + 1; }
__device__ __host__ int CeilAlign(int a, int b) { return CeilDiv(a, b) * b; }

__global__ void part1_init(const char *, int *, int);
__global__ void part1_kernel(const char *, int *, int, int);

void CountPosition(const char *text, int *pos, int text_size)
{
  const int maxK = 500;
  const int threadCount = 512;
  int blockCount = text_size / threadCount + 1;
  part1_init<<<blockCount, threadCount>>>(text, pos, text_size);
  cudaDeviceSynchronize();
  for (int i = 2; i <= maxK; ++i) {
    blockCount = (text_size / i + 1) / threadCount + 1;
    part1_kernel<<<blockCount, threadCount>>>(text, pos, text_size, i);
    cudaDeviceSynchronize();
  }
}

__global__ void part1_kernel(const char * text, int * pos, int n, int k) {
  int id = blockIdx.x * blockDim.x + threadIdx.x;
  id = id * k;
  /* should be careful here as id may int overflow */
  if (id >= n) {
    return;
  }
  if (pos[id] == 0) {
    return;
  }
  if (id > 0 && pos[id-1] == k-1) {
    pos[id] = k;
    return;
  }
  int i = pos[id];
  int j = id + (k - i - 1);
  if (j+1 < n && pos[j] == k-1 && text[j+1] != '\n') {
    pos[j+1] = k;
  }
}

__global__ void part1_init(const char * text, int * pos, int n) {
  int id = blockIdx.x * blockDim.x + threadIdx.x;
  if (id >= n) {
    return;
  }
  pos[id] = 0;
  if (text[id] != '\n') {
    pos[id] = -1;
    if (id == 0 || text[id-1] == '\n'){
      pos[id] = 1;
    }
  }
}

template<int I>
class equals {
public:
  __device__ bool operator () (int x) { return x == I;}
};

int ExtractHead(const int *pos, int *head, int text_size)
{
	int *buffer;
	int nhead;
	cudaMalloc(&buffer, sizeof(int)*text_size*2); // this is enough
	thrust::device_ptr<const int> pos_d(pos);
	thrust::device_ptr<int> head_d(head), flag_d(buffer), cumsum_d(buffer+text_size);

        auto head_end_d =
          thrust::copy_if(
            thrust::counting_iterator<int>(0),
            thrust::counting_iterator<int>(text_size),
            pos_d,
            head_d,
            equals<1>()
          );
        nhead = head_end_d - head_d;
	cudaFree(buffer);
	return nhead;
}

__global__ void part3_find_rpos(char *text, int n, int *pos, int *rpos);
__global__ void part3_string_flip(char *text, int n, int *pos, int *rpos);

void Part3(char *text, int *pos, int *head, int text_size, int n_head)
{
  int n = text_size;
  int *rpos;
  cudaMalloc(&rpos, n*sizeof(int));
  cudaMemset(rpos, 0, n*sizeof(int));
  int threadCount = 256;
  int blockCount = 1 + n / threadCount;
  part3_find_rpos<<<blockCount, threadCount>>>(text, n, pos, rpos);
  cudaDeviceSynchronize();
  part3_string_flip<<<blockCount, threadCount>>>(text, n, pos, rpos);
  cudaDeviceSynchronize();
}

__global__ void part3_find_rpos(char *text, int n, int *pos, int *rpos) {
  int ID = blockIdx.x * blockDim.x + threadIdx.x;
  if (ID >= n || pos[ID] == 0) {
    return;
  }
  int lb = ID, rb = ID+501;
  while (lb != rb-1) {
    int mid = (lb+rb) / 2;
    if (mid >= n) {
      rb = mid;
    } else if (mid - ID != pos[mid] - pos[ID]) {
      rb = mid;
    } else {
      lb = mid;
    }
  }
  rpos[ID] = lb - pos[ID] + 1;
}

__global__ void part3_string_flip(char *text, int n, int *pos, int *rpos) {
  int ID = blockIdx.x * blockDim.x + threadIdx.x;
  if (ID >= n || ID >= rpos[ID]) {
    return;
  }
  int pos1 = ID, pos2 = rpos[ID];
  char c = text[pos1];
  text[pos1] = text[pos2];
  text[pos2] = c;
}

