#include "lab2.h"
#include <iostream>
#include <cassert>

using namespace std;

#define CHECK {\
	auto e = cudaDeviceSynchronize();\
	if (e != cudaSuccess) {\
		printf("At " __FILE__ ":%d, %s\n", __LINE__, cudaGetErrorString(e));\
		abort();\
	}\
}

static const unsigned W = 854;
static const unsigned H = 480;
static const unsigned NFRAME = 24*60;
static const int permutation[] = { 151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

struct Lab2VideoGenerator::Impl {
  int t = 0;
  int blockWidth;
  dim3 threadDimension;
  dim3 blockDimension;
  double * R, * G, * B;
  int * perm;
};

Lab2VideoGenerator::Lab2VideoGenerator(): impl(new Impl) {
  impl->t = 0;
  int blockWidth = 16;
  impl->blockWidth = blockWidth;
  impl->threadDimension = dim3(blockWidth, blockWidth);
  impl->blockDimension =
    dim3(W / blockWidth + (W % blockWidth ? 1 : 0),
         H / blockWidth + (H % blockWidth ? 1 : 0));
  cudaMalloc(&impl->R, sizeof(double) * H * W);
  cudaMalloc(&impl->G, sizeof(double) * H * W);
  cudaMalloc(&impl->B, sizeof(double) * H * W);
  cudaMalloc(&impl->perm, sizeof(int) * 256);
  cudaMemcpy(impl->perm, permutation, sizeof(int) * 256, cudaMemcpyHostToDevice);
}

Lab2VideoGenerator::~Lab2VideoGenerator() {}

void Lab2VideoGenerator::get_info(Lab2VideoInfo &info) {
	info.w = W;
	info.h = H;
	info.n_frame = NFRAME;
	// fps = 24/1 = 24
	info.fps_n = 24;
	info.fps_d = 1;
};

__device__ __host__ double fade(double t) {
  // 6t^5 - 15t^4 + 10t^3
  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

__device__ __host__ double linearInterpolate(double l, double r, double s) {
  // s should be in [0, 1]
  return r * s + l * (1.0 - s);
}

__device__ __host__ double dotGradiant(int * p, int a, int b, int c, double dx, double dy, double dz) {
  int hashed = p[(p[(p[a & 255] + b) & 255] + c) & 255];
  switch (hashed % 12) {
    case 0x0: return dx + dy;
    case 0x1: return dx - dy;
    case 0x2: return dy - dx;
    case 0x3: return -dx - dy;
    case 0x4: return dx + dz;
    case 0x5: return dx - dz;
    case 0x6: return dz - dx;
    case 0x7: return -dx - dz;
    case 0x8: return dy + dz;
    case 0x9: return dy - dz;
    case 0xA: return dz - dy;
    case 0xB: return -dy - dz;
    // Should never reach here, but just in case
    default:  return dx + dy;
  }
}

__device__ __host__ double perlin(double x, double y, double z, int * p) {
  /* find boundary */
  int x0 = x;
  int y0 = y;
  int z0 = z;
  double xf = x - x0;
  double yf = y - y0;
  double zf = z - z0;
  double u = fade(xf);
  double v = fade(yf);
  double w = fade(zf);
  /* temparies to store intermediate result */
  double x1, x2, y1, y2;
  x1 = linearInterpolate(
        dotGradiant(p, x0, y0, z0, xf, yf, zf),
        dotGradiant(p, x0+1, y0, z0, xf-1, yf, zf),
        u
      );
  x2 = linearInterpolate(
        dotGradiant(p, x0, y0+1, z0, xf, yf-1, zf),
        dotGradiant(p, x0+1, y0+1, z0, xf-1, yf-1, zf),
        u
      );
  y1 = linearInterpolate(x1, x2, v);
  x1 = linearInterpolate(
        dotGradiant(p, x0, y0, z0+1, xf, yf, zf-1),
        dotGradiant(p, x0+1, y0, z0+1, xf-1, yf, zf-1),
        u
      );
  x2 = linearInterpolate(
        dotGradiant(p, x0, y0+1, z0+1, xf, yf-1, zf-1),
        dotGradiant(p, x0+1, y0+1, z0+1, xf-1, yf-1, zf-1),
        u
      );
  y2 = linearInterpolate(x1, x2, v);
  double z1 = linearInterpolate(y1, y2, w);
  // range is -1 ~ 1
  return z1;
}

__host__ __device__ double spin(double x, double y, double cx, double cy) {
  x = x - cx;
  y = y - cy;
  x = fabs(x);
  y = fabs(y);
  // special mossaic grid effect
  double d = x*x + y*y;
  if (sqrt(d) > 200) {
    if (y > x) return x / y;
    return y / x;
  } else {
    return d;
  }
}

__global__ void octavePerlin(double * R, double * G, double * B, int H, int W, int _t, int * perm) {
  int _x = blockIdx.x * blockDim.x + threadIdx.x;
  int _y = blockIdx.y * blockDim.y + threadIdx.y;
  if (_x >= W || _y >= H) return;
  double x, y, z;
  x = _x;
  y = _y;
  z = _t;
  z /= 24.0;
  // Octave perlin
  double ampl = 1.0;
  double freq = 0.02;
  double z1 = 0.0, w = 0.0;
  for (int i = 0; i < 7; ++i) {
    // Scale the coordinate
    z1 += ampl * fabs(perlin(x * freq, y * freq, z, perm));
    w += ampl;
    ampl *= 0.5;
    freq *= 2.0;
  }
  z1 /= w;
  int idx = _x + _y * W;
  R[idx] = sinf(z + z1 + x / 64);
  G[idx] = cosf(z1 + spin(x, y, W/2, H/2));
  B[idx] = sinf(z * 1.5 + z1 - x / 128);
}

__global__ void draw(double * R, double * G, double * B, int H, int W, uint8_t *yuv) {
  int _x = blockIdx.x * blockDim.x + threadIdx.x;
  int _y = blockIdx.y * blockDim.y + threadIdx.y;
  if (_x >= W || _y >= H) return;
  int idx = _x + _y * W;
  double r = R[idx], g = G[idx], b = B[idx];
  double Y, U, V;
  Y = +0.299 * r + 0.587 * g + 0.114 * b;
  U = -0.169 * r - 0.331 * g + 0.500 * b + 128;
  V = +0.500 * r - 0.419 * g - 0.081 * b + 128;
  yuv[idx] = 255 * Y;
  G[idx] = U;
  B[idx] = V;
  __syncthreads();
  if ((_x & 1) || (_y & 1)) {
    return;
  }
  // downsampling
  U = (G[idx] + G[idx+1] + G[idx+W] + G[idx+W+1]) / 4.0;
  V = (B[idx] + B[idx+1] + B[idx+W] + B[idx+W+1]) / 4.0;
  int _idx = _x / 2 + _y * W / 4;
  yuv[H * W + _idx] = 255 * U;
  yuv[H * W + (H * W / 4) + _idx] = 255 * V;
}

/*

  [ +0.299 +0.587 +0.114 +0   ]   [R]   [Y]
  [ -0.169 -0.331 +0.500 +128 ] x [G] = [U]
  [ +0.500 -0.419 -0.081 +128 ]   [B]   [V]
                                  [1]

  Y: [0, W*H)
  U: [W*H, W*H+W*H/4)
  V: [W*H+W*H/4, W*H+W*H/2)
 */

void Lab2VideoGenerator::Generate(uint8_t *yuv) {
  cudaMemset(yuv, 128, W*H + W*H/2);
  const auto& blockDimension = impl->blockDimension;
  const auto& threadDimension = impl->threadDimension;
  octavePerlin<<<blockDimension, threadDimension>>>(impl->R, impl->G, impl->B, H, W, impl->t, impl->perm);
  CHECK;
  draw<<<blockDimension, threadDimension>>>(impl->R, impl->G, impl->B, H, W, yuv);
  CHECK;
  ++(impl->t);
  return;
}

