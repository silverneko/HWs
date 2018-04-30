#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <queue>
#include <tuple>

#include "read_raw.h"

#define N 256

using namespace std;

Array2d<int> morphErosion(Array2d<int> F, Array2d<int> H);

// Usage: ./problem1a [infile] [outfile]
int main (int argc, char *argv[]) {
  assert(argc > 2);
  const char *infile = argv[1];
  string outfile(argv[2]);

  unsigned char data_raw[N][N], data_out[N][N];
  read_raw(infile, N*N, data_raw);

  Array2d<int> data_in(N, N);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      data_in(i, j) = data_raw[i][j] == 0xff;
    }
  }

  Array2d<int> H(3, 3);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      H(i, j) = 1;

  Array2d<int> E = morphErosion(data_in, H);
  Array2d<int> B(N, N);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      B(i, j) = data_in(i, j) - E(i, j);

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      data_out[i][j] = B(i, j) * 255;

  save_raw(outfile, N*N, data_out);

  return 0;
}

Array2d<int> morphErosion(Array2d<int> F, Array2d<int> H) {
  int S = 0;
  for (int i = 0; i < H.dim[0]; ++i)
    for (int j = 0; j < H.dim[1]; ++j)
      S += H(i, j);
  Array2d<int> G = filter(F, H);
  for (int i = 0; i < G.dim[0]; ++i)
    for (int j = 0; j < G.dim[1]; ++j)
      G(i, j) = G(i, j) == S;
  Array2d<int> pad_G = padwith(G, (H.dim[0]-1)/2, 0);
  return pad_G;
}
