#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <queue>
#include <tuple>

#include "read_raw.h"

using namespace std;

template<typename T>
Array2d<T> medianFilter(Array2d<T> F, int nw);
Array2d<int> morphErosion(Array2d<int> F, Array2d<int> H);
Array2d<int> morphDilation(Array2d<int> F, Array2d<int> H);

// Usage: ./problem1 [infile] [outfile]
int main (int argc, char *argv[]) {
  assert(argc > 2);
  const char *infile = argv[1];
  string outfile(argv[2]);

  const int W = 450, H = 248;
  unsigned char data_raw[H][W], data_out[H][W];
  read_raw(infile, H*W, data_raw);

  Array2d<int> data_in(H, W);
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j)
      data_in(i, j) = data_raw[i][j];

  // G = thresholded input
  Array2d<int> G(H, W);
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j)
      if (data_in(i, j) >= 100)
        G(i, j) = 0;
      else
        G(i, j) = 1;
  int K_pat[3][3] = {
    {1, 1, 0},
    {1, 1, 0},
    {0, 0, 0}
  };
  Array2d<int> K(3, 3);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      K(i, j) = K_pat[i][j];
  Array2d<int> F = morphDilation(morphErosion(G, K), K);

  char label[] =
    "ABCDEFGHIJKLMN"
    "OPQRSTUVWXYZab"
    "cdefghijklmnop"
    "qrstuvwxyz0123"
    "456789!@#$%^&*";
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 14; ++j) {
      int k = i*14+j;
      cout << label[k] << ',';
      cout << "48,32,";
      for (int ii = i*48; ii < (i+1)*48; ++ii)
        for (int jj = j*32; jj < (j+1)*32; ++jj)
          cout << F(ii, jj);
      cout << endl;
    }

  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j)
      if (i % 48 == 0 || j % 32 == 0)
        data_out[i][j] = 127;
      else {
        if (F(i, j) == 1)
          data_out[i][j] = 0;
        else
          data_out[i][j] = 255;
      }

  save_raw(outfile, H*W, data_out);

  return 0;
}

template<typename T>
Array2d<T> medianFilter(Array2d<T> F, int nw) {
  const int buf_size = nw*nw;
  const int hw = (nw-1) / 2;
  assert(hw * 2 + 1 == nw);
  Array2d<T> G(F.dim[0], F.dim[1]);
  for (int i = 0; i < F.dim[0]; ++i)
    for (int j = 0; j < F.dim[1]; ++j) {
      T buf[buf_size], bufN = 0;
      for (int ii = max(i-hw, 0); ii < min(i+hw+1, F.dim[0]); ++ii)
        for (int jj = max(j-hw, 0); jj < min(j+hw+1, F.dim[1]); ++jj)
          buf[bufN++] = F(ii, jj);
      sort(buf, buf+bufN);
      G(i, j) = buf[bufN / 2];
    }
  return G;
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
