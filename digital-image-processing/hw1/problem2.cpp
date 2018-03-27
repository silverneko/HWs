#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <random>

#include "read_raw.h"

#define N 256

using namespace std;

// clip v between [l, r]
double clip(double v, double l, double r) {
  return min(max(l, v), r);
}

void pad_edge(void *I, int ni, int nw, void *O);
void filter(void *I, int ni, void *K, int nk, void *O);
void med_filter(void *I, int ni, int nw, void *O);
double MSE(void *x, int nx, void *y);
double PSNR(void *x, int nx, void *y);

int main (int argc, char *argv[]) {
  unsigned char data_in[N][N];
  double I[N][N];

  read_raw("raw/sample3.raw", N*N, data_in);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      I[i][j] = data_in[i][j] / 255.0;
    }
  }

  // (a) Generate two noisy G1, G2 by adding guassian noise to I
  unsigned char G1[N][N], G2[N][N];
  {
    // standard deviation (sigma)
    double stddev1 = 0.02;
    double stddev2 = 0.05;
    mt19937 rng1(0x5EED), rng2(0xFFFF5EED);
    normal_distribution<double> dist1(0.0, stddev1), dist2(0.0, stddev2);
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        G1[i][j] = 255 * clip(I[i][j] + dist1(rng1), 0, 1);
        G2[i][j] = 255 * clip(I[i][j] + dist2(rng2), 0, 1);
      }
    }
  }
  save_raw("raw/problem2.G1.raw", N*N, G1);
  save_raw("raw/problem2.G2.raw", N*N, G2);

  // (b) Generate two noisy S1, S2 by adding s&p noise to I
  unsigned char S1[N][N], S2[N][N];
  {
    // threshold
    double th1 = 0.02;
    double th2 = 0.05;
    mt19937 rng(0x5EED);
    uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        double p = dist(rng);
        if (p < th1)
          S1[i][j] = 0;
        else if (p >= 1 - th1)
          S1[i][j] = 255;
        else
          S1[i][j] = data_in[i][j];
      }
    }
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        double p = dist(rng);
        if (p < th2)
          S2[i][j] = 0;
        else if (p >= 1 - th2)
          S2[i][j] = 255;
        else
          S2[i][j] = data_in[i][j];
      }
    }
  }
  save_raw("raw/problem2.S1.raw", N*N, S1);
  save_raw("raw/problem2.S2.raw", N*N, S2);

  // (c) Denoise G1, S1 and save result as RG, RS
  unsigned char RG[N][N], RS[N][N];
  double P[N+2][N+2], T[N][N], T2[N][N];
  double LP_kernel[3][3] = {
    1/16.0, 2/16.0, 1/16.0,
    2/16.0, 4/16.0, 2/16.0,
    1/16.0, 2/16.0, 1/16.0
  };
  double HP_kernel[3][3] = {
    0, -1, 0,
    -1, 5, -1,
    0, -1, 0
  };
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      T[i][j] = G1[i][j] / 255.0;
  pad_edge(T, N, 1, P);
  filter(P, N+2, LP_kernel, 3, T);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      RG[i][j] = 255 * clip(T[i][j], 0, 1);
  save_raw("raw/problem2.RG.raw", N*N, RG);

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      T[i][j] = S1[i][j] / 255.0;
  med_filter(T, N, 1, T2);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      RS[i][j] = 255 * clip(T2[i][j], 0, 1);
  save_raw("raw/problem2.RS.raw", N*N, RS);

  // (d) Compute PSNR of RG, RS
  printf("PSNR of RG: %.6lf dB\n", PSNR(RG, N*N, data_in));
  printf("PSNR of RS: %.6lf dB\n", PSNR(RS, N*N, data_in));


  // (II) Remove wrinkles
  unsigned char R[N][N];
  read_raw("raw/sample4.raw", N*N, data_in);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      I[i][j] = data_in[i][j] / 255.0;
    }
  }
  /*
  double g_kernel[5][5] = {
    1, 2, 4, 2, 1,
    2, 4, 8, 4, 2,
    4, 8, 16, 8, 4,
    2, 4, 8, 4, 2,
    1, 2, 4, 2, 1
  };
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
      g_kernel[i][j] /= 100.0;
  double P2[N+4][N+4];
  pad_edge(I, N, 2, P2);
  filter(P2, N+4, g_kernel, 5, T);
  */
  pad_edge(I, N, 1, P);
  filter(P, N+2, LP_kernel, 3, T2);
  med_filter(I, N, 2, T);
  double alpha = 1;
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      T[i][j] = (T2[i][j] + alpha * T[i][j]) / (1 + alpha);

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      R[i][j] = 255 * clip(T[i][j], 0, 1);
  save_raw("raw/problem2.II.raw", N*N, R);

  return 0;
}

#define F(x, y) (((double*)I)[ni*(x)+(y)])
#define G(x, y) (((double*)O)[no*(x)+(y)])
#define K(x, y) (((double*)kernel)[nk*(x)+(y)])

void pad_edge(void *I, int ni, int nw, void *O) {
  int no = nw + ni + nw;
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

void med_filter(void *I, int ni, int nw, void *O) {
  int no = ni;
  int buf_size = (2*nw+1)*(2*nw+1);
  for (int i = 0; i < ni; ++i) {
    for (int j = 0; j < ni; ++j) {
      double buf[buf_size];
      int l = 0;
      for (int ii = max(i-nw, 0); ii <= min(i+nw, ni-1); ++ii) {
        for (int jj = max(j-nw, 0); jj <= min(j+nw, ni-1); ++jj) {
          buf[l++] = F(ii, jj);
        }
      }
      sort(buf, buf+l);
      G(i, j) = l & 1 ? buf[l / 2] : (buf[l / 2 - 1] + buf[l / 2]) / 2;
    }
  }
}

double MSE(void *x, int nx, void *y) {
  double res = 0;
  for (int i = 0; i < nx; ++i) {
    int a = ((unsigned char*)x)[i];
    int b = ((unsigned char*)y)[i];
    res += (a-b)*(a-b);
  }
  res /= nx;
  return res;
}

double PSNR(void *x, int nx, void *y) {
  return 10 * log10(255.0*255.0 / MSE(x, nx, y));
}
