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

void matmul(void *A, int n, int m, void *B, int k, void *C);
void matinv(void *A, int n, void *B);

// Usage: ./problem2b [infile] [outfile]
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

  int n = 22, m = 21;
  float Y[n][2] = {
    {0, 0},
    {0, 156},
    {0, 227},
    {0, 298},
    {0, 369},
    {0, 440},
    {0, 511},
    {111, 511},
    {211, 511},
    {311, 511},
    {411, 511},
    {511, 511},
    {511, 440},
    {511, 369},
    {511, 298},
    {511, 227},
    {511, 156},
    {511, 85},
    {511, 0},
    {411, 0},
    {311, 0},
    {211, 0}
  };
  float px[n][2] = {
    {-20, -20},
    {10, 139},
    {-10, 211},
    {10, 283},
    {-10, 355},
    {10, 427},
    {-10, 499},
    {100, 489},
    {200, 511},
    {300, 489},
    {400, 511},
    {500, 480},
    {511, 427},
    {491, 355},
    {511, 283},
    {491, 211},
    {511, 139},
    {491, 68},
    {511, -10},
    {400, 10},
    {300, -10},
    {200, 10}
  };

  float X[n][m], XT[m][n];
  for (int i = 0; i < n; ++i) {
    float x = px[i][0], y = px[i][1];
    int pos = 1;
    X[i][0] = 1;
    for (int j = 1; j <= 5; ++j) {
      for (int k = 0; k < j; ++k)
        X[i][pos+k] = X[i][pos-j+k] * x;
      X[i][pos+j] = X[i][pos-1] * y;
      pos += j+1;
    }
  }
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j)
      XT[j][i] = X[i][j];

  float XTX[m][m], XTXinv[m][m], B[m][n], M[m][2];
  matmul(XT, m, n, X, m, XTX);
  matinv(XTX, m, XTXinv);
  matmul(XTXinv, m, m, XT, n, B);
  matmul(B, m, n, Y, 2, M);
  // M is the reverse wrap coefficient
  // G(x, y) = F(u, v)
  // where (u, v) = M x P(x, y), F = input, G = output, P = polynomial

  unsigned char out[N][N];
  save_raw(outfile + ".raw", N*N, out);

  return 0;
}

void matmul(void *A, int n, int m, void *B, int k, void *C);
void matinv(void *A, int n, void *B);

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
