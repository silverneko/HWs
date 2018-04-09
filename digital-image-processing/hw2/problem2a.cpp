#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <queue>
#include <tuple>

#include "read_raw.h"

#define N 512

using namespace std;

float clip(float v, float l, float r) {
  return min(max(l, v), r);
}

void pad(void *I, int ni, int nw, void *O);
void filter(void *I, int ni, void *K, int nk, void *O);

// Usage: ./problem2a [infile] [outfile]
int main (int argc, char *argv[]) {
  assert(argc > 2);
  const char *infile = argv[1];
  string outfile(argv[2]);

  unsigned char data_raw[N][N];
  read_raw(infile, N*N, data_raw);

  float data_in[N][N];
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      data_in[i][j] = data_raw[i][j] / 255.0;

  float gaussian5[5][5] = {
    {2, 4, 5, 4, 2},
    {4, 9, 12, 9, 4},
    {5, 12, 15, 12, 5},
    {4, 9, 12, 9, 4},
    {2, 4, 5, 4, 2}
  };
  float gaussian3[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
  };

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      gaussian3[i][j] /= 16;
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
      gaussian5[i][j] /= 159;

  unsigned char out[N][N];
  float data_p[N+4][N+4], data_lp[N][N], unsharp[N][N];
  float c = 0.7;
  float a = c / (2*c - 1), b = (1 - c) / (2*c - 1);

  pad(data_in, N, 1, data_p);
  filter(data_p, N+2, gaussian3, 3, data_lp);

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      unsharp[i][j] = a * data_in[i][j] - b * data_lp[i][j];

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      out[i][j] = 255 * clip(unsharp[i][j], 0, 1);

  save_raw(outfile + ".raw", N*N, out);

  return 0;
}

#define F(x, y) (((float*)I)[ni * (x) + (y)])
#define G(x, y) (((float*)O)[no * (x) + (y)])
#define K(x, y) (((float*)kernel)[nk * (x) + (y)])

void pad(void *I, int ni, int nw, void *O) {
  int no = ni + nw * 2;
  for (int i = 0; i < ni; ++i) {
    for (int j = 0; j < ni; ++j) {
      G(i+nw, j+nw) = F(i, j);
    }
  }
  for (int i = 0; i < nw; ++i) {
    for (int j = 0; j < ni; ++j) {
      G(i, j+nw) = G(nw, j+nw);
      G(nw + ni + i, j+nw) = G(nw+ni-1, j+nw);
      G(j+nw, i) = G(j+nw, nw);
      G(j+nw, nw + ni + i) = G(j+nw, nw+ni-1);
    }
  }
  for (int i = 0; i < nw; ++i) {
    for (int j = 0; j < nw; ++j) {
      G(i, j) = G(nw, nw);
      G(nw+ni+i, j) = G(nw+ni-1, nw);
      G(i, nw+ni+j) = G(nw, nw+ni-1);
      G(nw+ni+i, nw+ni+j) = G(nw+ni-1, nw+ni-1);
    }
  }
}

void filter(void *I, int ni, void *kernel, int nk, void *O) {
  int no = ni-(nk-1);
  for (int i = 0; i < no; ++i) {
    for (int j = 0; j < no; ++j) {
      double accu = 0;
      for (int ik = 0; ik < nk; ++ik) {
        for (int jk = 0; jk < nk; ++jk) {
          accu += K(ik, jk) * F(i+ik, j+jk);
        }
      }
      G(i, j) = accu;
    }
  }
}
