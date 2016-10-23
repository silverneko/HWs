#include <cstdio>
#include <functional>

#include "lab3.h"

#ifdef DEBUG
#include "Timer.h"
#endif

using namespace std;

__device__ __host__ int CeilDiv(int a, int b) { return (a-1)/b + 1; }
__device__ __host__ int CeilAlign(int a, int b) { return CeilDiv(a, b) * b; }

__global__ void SimpleClone(
  const float *background,
  const float *target,
  const float *mask,
  float *output,
  const int wb, const int hb, const int wt, const int ht,
  const int oy, const int ox
  )
{
  const int yt = blockIdx.y * blockDim.y + threadIdx.y;
  const int xt = blockIdx.x * blockDim.x + threadIdx.x;
  const int curt = wt*yt+xt;
  if (yt < ht and xt < wt and mask[curt] > 127.0f) {
    const int yb = oy+yt, xb = ox+xt;
    const int curb = wb*yb+xb;
    if (0 <= yb and yb < hb and 0 <= xb and xb < wb) {
      output[curb*3+0] = target[curt*3+0];
      output[curb*3+1] = target[curt*3+1];
      output[curb*3+2] = target[curt*3+2];
    }
  }
}

class RGB {
public:
  float * R, * G, * B;
};

__global__ void calculateFixed(
  RGB background,
  RGB target,
  const float * mask,
  RGB output,
  const int wb, const int hb, const int wt, const int ht,
  const int oy, const int ox
  )
{
  const int idx = blockDim.x * blockIdx.x + threadIdx.x;
  const int idy = blockDim.y * blockIdx.y + threadIdx.y;
  const int curt = wt * idy + idx;
  if (idx >= wt || idy >= ht || mask[curt] < 127.0f) {
    return;
  }
  float result[3];
  result[0] = 4*target.R[curt];
  result[1] = 4*target.G[curt];
  result[2] = 4*target.B[curt];
  const int dir[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  for (int i = 0; i < 4; ++i) {
    const int nx = idx + dir[i][0];
    const int ny = idy + dir[i][1];
    const int curn = wt*ny + nx;
    if (nx >= 0 && ny >= 0 && nx < wt && ny < ht) {
      result[0] -= target.R[curn];
      result[1] -= target.G[curn];
      result[2] -= target.B[curn];
    } else {
      result[0] -= target.R[curt];
      result[1] -= target.G[curt];
      result[2] -= target.B[curt];
    }
    if ((nx < 0 || ny < 0 || nx >= wt || ny >= ht) || mask[curn] < 127.0f) {
      const int bx = nx + ox;
      const int by = ny + oy;
      const int curb = wb * by + bx;
      result[0] += background.R[curb];
      result[1] += background.G[curb];
      result[2] += background.B[curb];
    }
  }
  output.R[curt] = result[0];
  output.G[curt] = result[1];
  output.B[curt] = result[2];
}

__global__ void calculateJacobi(
  RGB fixed,
  const float * mask,
  RGB target,
  RGB output,
  const int wt,
  const int ht
  )
{
  const int idx = blockDim.x * blockIdx.x + threadIdx.x;
  const int idy = blockDim.y * blockIdx.y + threadIdx.y;
  const int curt = wt * idy + idx;
  if (idx >= wt || idy >= ht || mask[curt] < 127.0f) {
    return;
  }
  float result[3];
  result[0] = fixed.R[curt];
  result[1] = fixed.G[curt];
  result[2] = fixed.B[curt];
  const int dir[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  for (int i = 0; i < 4; ++i) {
    const int nx = idx + dir[i][0];
    const int ny = idy + dir[i][1];
    const int curn = wt*ny + nx;
    if (nx >= 0 && nx < wt && ny >= 0 && ny < ht && mask[curn] > 127.0f) {
      result[0] += target.R[curn];
      result[1] += target.G[curn];
      result[2] += target.B[curn];
    }
  }
  output.R[curt] = result[0] / 4;
  output.G[curt] = result[1] / 4;
  output.B[curt] = result[2] / 4;
}

__global__ void transpose2RGBinternal(const float * ch, int nsize, RGB rgb) {
  const int id = blockDim.x * blockIdx.x + threadIdx.x;
  if (id >= nsize) {
    return;
  }
  rgb.R[id] = ch[3*id + 0];
  rgb.G[id] = ch[3*id + 1];
  rgb.B[id] = ch[3*id + 2];
}

__global__ void transposeFromRGBinternal(RGB rgb, int nsize, float * ch) {
  const int id = blockDim.x * blockIdx.x + threadIdx.x;
  if (id >= nsize) {
    return;
  }
  ch[3*id + 0] = rgb.R[id];
  ch[3*id + 1] = rgb.G[id];
  ch[3*id + 2] = rgb.B[id];
}

void transpose2RGB(const float * ch, int nsize, RGB rgb) {
  transpose2RGBinternal<<<CeilDiv(nsize, 512), 512>>>(ch, nsize, rgb);
}

void transposeFromRGB(RGB rgb, int nsize, float * ch) {
  transposeFromRGBinternal<<<CeilDiv(nsize, 512), 512>>>(rgb, nsize, ch);
}

void newRGB(RGB * rgb, int nsize) {
  cudaMalloc(&rgb->R, nsize * sizeof(float));
  cudaMalloc(&rgb->G, nsize * sizeof(float));
  cudaMalloc(&rgb->B, nsize * sizeof(float));
}

void freeRGB(RGB rgb) {
  cudaFree(rgb.R);
  cudaFree(rgb.G);
  cudaFree(rgb.B);
}

void PoissonImageCloning(
  const float *background,
  const float *target,
  const float *mask,
  float *output,
  const int wb, const int hb, const int wt, const int ht,
  const int oy, const int ox
  )
{
  const int nsize = wt * ht;
  RGB bgRGB, fgRGB;
  newRGB(&bgRGB, wb * hb);
  newRGB(&fgRGB, nsize);
  transpose2RGB(background, wb * hb, bgRGB);
  transpose2RGB(target, nsize, fgRGB);

  RGB fixed, buf1, buf2;
  newRGB(&fixed, nsize);
  newRGB(&buf1, nsize);
  newRGB(&buf2, nsize);

#ifdef DEBUG
  /* TIMER */
  cudaDeviceSynchronize();
  Timer timer;
  timer.Start();
  /* TIMER */
#endif

  dim3 gridDimension(CeilDiv(wt, 32), CeilDiv(ht, 16)), blockDimension(32, 16);
  calculateFixed <<<gridDimension, blockDimension>>> (
    bgRGB, fgRGB, mask, fixed, wb, hb, wt, ht, oy, ox
  );

  cudaMemcpy(buf1.R, fgRGB.R, nsize * sizeof(float), cudaMemcpyDeviceToDevice);
  cudaMemcpy(buf1.G, fgRGB.G, nsize * sizeof(float), cudaMemcpyDeviceToDevice);
  cudaMemcpy(buf1.B, fgRGB.B, nsize * sizeof(float), cudaMemcpyDeviceToDevice);
  for (int i = 0; i < 20000; ++i) {
    calculateJacobi <<<gridDimension, blockDimension>>> (
      fixed, mask, buf1, buf2, wt, ht
    );
    swap(buf1, buf2);
  }

  float * buf3;
  cudaMalloc(&buf3, 3 * nsize * sizeof(float));
  transposeFromRGB(buf1, nsize, buf3);
  cudaMemcpy(output, background, wb*hb*sizeof(float)*3, cudaMemcpyDeviceToDevice);
  SimpleClone <<<gridDimension, blockDimension>>>(
    background, buf3, mask, output,
    wb, hb, wt, ht, oy, ox
  );

#ifdef DEBUG
  /* TIMER */
  cudaDeviceSynchronize();
  timer.Pause();
  printf_timer(timer);
  /* TIMER */
#endif

  cudaFree(buf3);
  freeRGB(buf2);
  freeRGB(buf1);
  freeRGB(fixed);
  freeRGB(fgRGB);
  freeRGB(bgRGB);
}
