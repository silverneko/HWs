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

int main (int argc, char *argv[]) {
  unsigned char data_in[N][N];
  read_raw("raw/sample3.raw", N*N, data_in);
  double I[N][N];
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      I[i][j] = data_in[i][j] / 255.0;
    }
  }

  // (a) Generate two noisy G1, G2 by adding guassian noise to I3
  {
    // standard deviation (sigma)
    double stddev1 = 0.01;
    double stddev2 = 0.1;
    mt19937 rng1(0x5EED), rng2(0xFFFF5EED);
    normal_distribution<double> dist1(0.0, stddev1), dist2(0.0, stddev2);
    unsigned char G1[N][N], G2[N][N];
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        G1[i][j] = 255 * clip(I[i][j] + dist1(rng1), 0, 1);
        G2[i][j] = 255 * clip(I[i][j] + dist2(rng2), 0, 1);
      }
    }

    save_raw("raw/problem2.G1.raw", N*N, G1);
    save_raw("raw/problem2.G2.raw", N*N, G2);
  }

  return 0;
}
