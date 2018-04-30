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

Array2d<int> morphDilation(Array2d<int> F, Array2d<int> H);

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
  fill_n(H[0], 3*3, 1);

  Array2d<int> label(N, N);
  fill_n(label[0], N*N, 0);

  int cid = 0;
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      if (label(i, j) != 0 || data_in(i, j) == 0)
        continue;
      ++cid;
      Array2d<int> G(N, N);
      fill_n(G[0], N*N, 0);
      G(i, j) = 1;
      int G_sum = 1;

      while (true) {
        Array2d<int> G_ = morphDilation(G, H);
        int G_sum_ = 0;
        for (int x = 0; x < N; ++x)
          for (int y = 0; y < N; ++y) {
            G_(x, y) = G_(x, y) * data_in(x, y);
            G_sum_ += G_(x, y);
          }
        if (G_sum == G_sum_) {
          break;
        }
        G_sum = G_sum_;
        G = G_;
      }

      for (int x = 0; x < N; ++x)
        for (int y = 0; y < N; ++y)
          if (G(x, y) == 1) {
            label(x, y) = cid;
          }
    }



  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      data_out[i][j] = label(i, j) * 1.0 / cid * 255;

  save_raw(outfile, N*N, data_out);

  cout << infile << " consists of " << cid << " connected components.\n";

  return 0;
}

Array2d<int> morphDilation(Array2d<int> F, Array2d<int> H) {
  Array2d<int> pad_F = padwith(F, (H.dim[0]-1)/2, 0);
  Array2d<int> K(H.dim[0], H.dim[1]);
  // K == mirrored H
  // so that filter(F, K) == convolution(F, H)
  for (int i = 0; i < H.dim[0]; ++i)
    for (int j = 0; j < H.dim[1]; ++j)
      K(i, j) = H(H.dim[0]-1-i, H.dim[1]-1-j);
  Array2d<int> G = filter(pad_F, K);
  for (int i = 0; i < G.dim[0]; ++i)
    for (int j = 0; j < G.dim[1]; ++j)
      G(i, j) = G(i, j) != 0;
  return G;
}
