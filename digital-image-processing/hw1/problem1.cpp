#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "read_raw.h"

#define N 256

using namespace std;

void histogram(void *buf, unsigned int size, unsigned int *hist) {
  for (int i = 0; i < 256; ++i)
    hist[i] = 0;
  for (int i = 0; i < size; ++i) {
    unsigned char I = ((unsigned char*)(buf))[i];
    hist[I]++;
  }
  return;
}

void power_law(double p, double *buf, unsigned int size, double *out);
void log_transform(double a, double *buf, unsigned int size, double *out);
void inv_log_transform(double a, double *buf, unsigned int size, double *out);

int main (int argc, char *argv[]) {
  unsigned char data_in[N][N];
  read_raw("raw/sample2.raw", N*N, data_in);

  // (a) Output D which is input / 3
  unsigned char data_D[N][N];
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      data_D[i][j] = data_in[i][j] / 3;
    }
  }

  save_raw("raw/problem1.D.raw", N*N, data_D);

  // (b) Output histogram
  unsigned int hist1[256], hist2[256];
  histogram(data_in, N*N, hist1);
  histogram(data_D, N*N, hist2);
  printf("histogram of I2:\n");
  for (int i = 0; i < 256; ++i)
    printf("%d ", hist1[i]);
  printf("\n");
  printf("histogram of D:\n");
  for (int i = 0; i < 256; ++i)
    printf("%d ", hist2[i]);
  printf("\n");

  // (c) Histogram equalization
  unsigned int CDF[256];
  CDF[0] = hist2[0];
  for (int i = 1; i < 256; ++i) {
    CDF[i] = CDF[i-1] + hist2[i];
  }
  for (int i = 0; i < 256; ++i) {
    CDF[i] = 255.0 * CDF[i] / CDF[255];
  }
  unsigned char data_H[N][N];
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      data_H[i][j] = CDF[data_D[i][j]];
    }
  }
  save_raw("raw/problem1.H.raw", N*N, data_H);

  // (d) Local histo eq
  int radius = 30;
  unsigned char data_L[N][N];
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      unsigned int hist[256] = {0};
      for (int dx = max(i-radius, 0); dx <= min(i+radius, N-1); ++dx) {
	for (int dy = max(j-radius, 0); dy <= min(j+radius, N-1); ++dy) {
	  hist[data_D[dx][dy]]++;
	}
      }
      CDF[0] = hist[0];
      for (int i = 1; i < 256; ++i)
	CDF[i] = CDF[i-1] + hist[i];
      for (int i = 0; i < 256; ++i)
	CDF[i] = 255.0 * CDF[i] / CDF[255];
      data_L[i][j] = CDF[data_D[i][j]];
    }
  }
  save_raw("raw/problem1.L.raw", N*N, data_L);

  // (e) Histogram of H and L
  histogram(data_H, N*N, hist1);
  histogram(data_L, N*N, hist2);
  printf("histogram of H:\n");
  for (int i = 0; i < 256; ++i)
    printf("%d ", hist1[i]);
  printf("\n");
  printf("histogram of L:\n");
  for (int i = 0; i < 256; ++i)
    printf("%d ", hist2[i]);
  printf("\n");

  // (f) enhance D
  double data_in_scaled[N*N];
  double data_out[N*N];
  unsigned char data_f[N*N];
  for (int i = 0; i < N*N; ++i)
    data_in_scaled[i] = ((unsigned char*)data_D)[i] / 255.0;
  // log
  log_transform(2.5, data_in_scaled, N*N, data_out);
  for (int i = 0; i < N*N; ++i) {
    data_f[i] = min(255, max(0, int(255 * data_out[i])));
  }
  save_raw("raw/problem1.f.log.raw", N*N, data_f);
  histogram(data_f, N*N, hist1);
  printf("histogram of log:\n");
  for (int i = 0; i < 256; ++i)
    printf("%d ", hist1[i]);
  printf("\n");
  // inv log
  inv_log_transform(log(10), data_in_scaled, N*N, data_out);
  for (int i = 0; i < N*N; ++i)
    data_f[i] = min(255, max(0, int(255 * data_out[i])));
  save_raw("raw/problem1.f.invlog.raw", N*N, data_f);
  histogram(data_f, N*N, hist1);
  printf("histogram of invlog:\n");
  for (int i = 0; i < 256; ++i)
    printf("%d ", hist1[i]);
  printf("\n");
  // power law
  power_law(0.7, data_in_scaled, N*N, data_out);
  for (int i = 0; i < N*N; ++i)
    data_f[i] = min(255, max(0, int(255 * data_out[i])));
  save_raw("raw/problem1.f.powlaw.raw", N*N, data_f);
  histogram(data_f, N*N, hist1);
  printf("histogram of power law:\n");
  for (int i = 0; i < 256; ++i)
    printf("%d ", hist1[i]);
  printf("\n");

  return 0;
}

void power_law(double p, double *buf, unsigned int size, double *out) {
  for (int i = 0; i < size; ++i)
    out[i] = pow(buf[i], p);
  return;
}

void log_transform(double a, double *buf, unsigned int size, double *out) {
  for (int i = 0; i < size; ++i)
    out[i] = log(1 + a * buf[i]) / log(2);
  return;
}

void inv_log_transform(double a, double *buf, unsigned int size, double *out) {
  for (int i = 0; i < size; ++i)
    out[i] = exp(a * buf[i]) - 1;
  return;
}
