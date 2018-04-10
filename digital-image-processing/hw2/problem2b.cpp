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

void polynomial(double x, double y, double *p);
void matmul(void *A, int n, int m, void *B, int r, void *C);
void linearSolve(void *A, int n, void *Y, int m, void *X);

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

  float G[N][N];
  float dx = 8, dy = 8;
  float sx = 511.0 / 500, sy = 511.0 / 500;
  float xa = 20, ya = 20;
  float xcycle = 2 * M_PI / 160;
  float ycycle = 2 * M_PI / 250;
  float xphase = 0;
  float yphase = 0;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      float u = sx * (dx + i + xa * sin(xphase + xcycle * j));
      float v = sy * (dy + j + ya * sin(yphase + ycycle * i));

      if (u < 0 || u > N-1 || v < 0 || v > N-1) {
        G[i][j] = 0;
        continue;
      }
      int x = ceil(u), y = ceil(v);
      float a = u - x, b = v - y;
#define interp(l, r, a) ((l) * (1.0 - (a)) + (r) * (a))
      float t1 = interp(data_in[x][y], data_in[x+1][y], a);
      float t2 = interp(data_in[x][y+1], data_in[x+1][y+1], a);
      G[i][j] = interp(t1, t2, b);
      G[i][j] = data_in[x][y];
    }
  }

  unsigned char out[N][N];
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      out[i][j] = 255 * clip(G[i][j], 0, 1);
  save_raw(outfile + ".raw", N*N, out);
  /*
  int n = 23, m = 21;
  double Y[n][2] = {
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
    {211, 0},
    {111, 0}
  };
  double px[n][2] = {
    {-50, -50},
    {50, 139},
    {-50, 211},
    {50, 283},
    {-50, 355},
    {50, 427},
    {-50, 499},
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
    {511, -50},
    {400, 50},
    {300, -50},
    {200, 50},
    {100, -50}
  };

  double X[n][m], XT[m][n];
  for (int i = 0; i < n; ++i) {
    double x = px[i][0], y = px[i][1];
    polynomial(x, y, X[i]);
  }
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j)
      XT[j][i] = X[i][j];

  double XTX[m][m], XTY[m][2], M[m][2];
  matmul(XT, m, n, X, m, XTX);
  matmul(XT, m, n, Y, 2, XTY);
  linearSolve(XTX, m, XTY, 2, M);

  // M is the reverse wrap coefficient
  // G(x, y) = F(u, v)
  // where (u, v) = M x P(x, y), F = input, G = output, P = polynomial
  float G[N][N];
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      double p[m], uv[2];
      polynomial(i, j, p);
      matmul(p, 1, m, M, 2, uv);
      double u = uv[0], v = uv[1];
      if (u < 0 || u > N-1 || v < 0 || v > N-1) {
        G[i][j] = 0;
        continue;
      }
      // G[i][j] = 1;
      G[i][j] = data_in[int(u)][int(v)];
    }
  }

  unsigned char out[N][N];
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      out[i][j] = 255 * clip(G[i][j], 0, 1);
  save_raw(outfile + ".raw", N*N, out);
  */
  return 0;
}

void polynomial(double x, double y, double *p) {
  int pos = 1;
  /*
  double a[4] = {1, x, x*x, x*x*x};
  double b[4] = {1, y, y*y, y*y*y};
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      p[i*4+j] = a[i]*b[j];
  */
  p[0] = 1;
  for (int j = 1; j <= 5; ++j) {
    for (int k = 0; k < j; ++k)
      p[pos+k] = p[pos-j+k] * x;
    p[pos+j] = p[pos-1] * y;
    pos += j+1;
  }
}

void matmul(void *A, int n, int m, void *B, int r, void *C) {
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < r; ++j) {
      double accu = 0;
      for (int k = 0; k < m; ++k)
        // accu += A[i][k] * B[k][j];
        accu += ((double*)A)[i * m + k] * ((double*)B)[k * r + j];
      // C[i][j] = accu;
      ((double*)C)[i * r + j] = accu;
    }
}

// solve AX = Y for X
void linearSolve(void *A_, int n, void *Y_, int m, void *X_) {
#define A(i, j) (((double*)A_)[n * (i) + (j)])
#define X(i, j) (((double*)X_)[m * (i) + (j)])
#define Y(i, j) (((double*)Y_)[m * (i) + (j)])
  // first do pivoted LU factorization on A
  int piv[n];
  for (int k = 0; k < n-1; ++k) {
    int p = k;
    for (int i = k+1; i < n; ++i)
      if (fabs(A(i, k)) > fabs(A(p, k)))
        p = i;
    piv[k] = p;
    for (int i = 0; i < n; ++i)
      swap(A(k, i), A(piv[k], i));
    if (A(k, k) != 0) {
      for (int i = k+1; i < n; ++i)
        A(i, k) = A(i, k) / A(k, k);
      for (int i = k+1; i < n; ++i)
        for (int j = k+1; j < n; ++j)
          A(i, j) = A(i, j) - A(i, k) * A(k, j);
    }
  }
  // PA = LU, solve LUX = PY
  for (int i = 0; i < n-1; ++i)
    for (int j = 0; j < m; ++j)
      swap(Y(i, j), Y(piv[i], j));
  // first solve LB = PY for B, store B in X
  for (int j = 0; j < m; ++j)
    for (int i = 0; i < n; ++i) {
      double accu = 0;
      for (int k = 0; k < i; ++k)
        accu += A(i, k) * X(k, j);
      X(i, j) = Y(i, j) - accu;
    }
  // next solve UX = B for X, B is stored in X, so update X inplace
  for (int j = 0; j < m; ++j)
    for (int i = n-1; i >= 0; --i) {
      double accu = 0;
      for (int k = i+1; k < n; ++k)
        accu += A(i, k) * X(k, j);
      X(i, j) = (X(i, j) - accu) / A(i, i);
    }
#undef A
#undef X
#undef Y
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
