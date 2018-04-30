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

Array2d<int> pixelStacker(Array2d<int> F);

// Usage: ./problem1a [infile] [outfile]
int main (int argc, char *argv[]) {
  assert(argc > 2);
  const char *infile = argv[1];
  string outfile(argv[2]);

  unsigned char data_raw[N][N], data_out[N][N];
  read_raw(infile, N*N, data_raw);

  Array2d<int> data_in(N, N), F(N, N);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      data_in(i, j) = data_raw[i][j] == 0xff;
      F(i, j) = data_in(i, j);
    }
  }

  int stage1pattern[] = {
    0b01010000,
    0b00010100,
    0b00000101,
    0b01000001,
    0b11100000,
    0b00111000,
    0b00001110,
    0b10000011,
    0b11110000,
    0b00111100,
    0b00001111,
    0b11000011,
    0b01111000,
    0b00011110,
    0b10000111,
    0b11100001,
    0b11111000,
    0b00111110,
    0b10001111,
    0b11100011,
    0b01111100,
    0b00011111,
    0b11000111,
    0b11110001,
    0b11111100,
    0b01111110,
    0b00111111,
    0b10011111,
    0b11001111,
    0b11100111,
    0b11110011,
    0b11111001,
    0b01111111,
    0b10111111,
    0b11011111,
    0b11101111,
    0b11110111,
    0b11111011,
    0b11111101,
    0b11111110
  };
  int stage2pattern[] = {
    0b10000000,
    0b00100000,
    0b00001000,
    0b00000010,
    0b01000000,
    0b00010000,
    0b00000100,
    0b00000001,
    0b01010000,
    0b00010100,
    0b00000101,
    0b01000001
  };
  int stage2partialpattern[] = {
    0b01110000,
    0b00011100,
    0b00000111,
    0b11000001,
    0b01010001,
    0b01000101,
    0b00010101,
    0b01010100
  };

  bool stage1mask[256] = {0};
  bool stage2mask[256] = {0};
  for (int m : stage1pattern)
    stage1mask[m] = true;
  for (int m : stage2pattern)
    stage2mask[m] = true;
  for (int i = 0; i < 256; ++i) {
    for (int m : stage2partialpattern)
      if ((i & m) == m)
        stage2mask[i] = true;
    if ((i & 0b10100000) == 0b10100000 && (i & 0b00001110) != 0)
      stage2mask[i] = true;
    if ((i & 0b10000010) == 0b10000010 && (i & 0b00111000) != 0)
      stage2mask[i] = true;
    if ((i & 0b00001010) == 0b00001010 && (i & 0b11100000) != 0)
      stage2mask[i] = true;
    if ((i & 0b00101000) == 0b00101000 && (i & 0b10000011) != 0)
      stage2mask[i] = true;
    if ((i & 0b01010010) == 0b01010010 && (i & 0b00100101) == 0)
      stage2mask[i] = true;
    if ((i & 0b10010100) == 0b10010100 && (i & 0b01001001) == 0)
      stage2mask[i] = true;
    if ((i & 0b00100101) == 0b00100101 && (i & 0b01010010) == 0)
      stage2mask[i] = true;
    if ((i & 0b01001001) == 0b01001001 && (i & 0b10010100) == 0)
      stage2mask[i] = true;
  }

  while (true) {
    Array2d<int> FS = pixelStacker(F);
    Array2d<int> M(N, N);
    fill_n(M[0], N*N, 0);
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j) {
        if (F(i, j) && stage1mask[FS(i, j)]) {
          M(i, j) = 1;
        }
      }
    bool changed = false;
    Array2d<int> MS = pixelStacker(M);
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j) {
        if (M(i, j) && !stage2mask[MS(i, j)]) {
          F(i, j) = 0;
          changed = true;
        }
      }
    if (!changed)
      break;
  }

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      data_out[i][j] = F(i, j) * 255;

  save_raw(outfile, N*N, data_out);

  return 0;
}

/* 7 6 5
 * 0 x 4
 * 1 2 3
 */
Array2d<int> pixelStacker(Array2d<int> F) {
  int bit_seq[8][2]
    = {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 1}, {2, 0}, {1, 0}};
  Array2d<int> G(F.dim[0], F.dim[1]);
  Array2d<int> F_pad = padwith(F, 1, 0);
  for (int i = 0; i < G.dim[0]; ++i)
    for (int j = 0; j < G.dim[1]; ++j) {
      int s = 0;
      for (int k = 0; k < 8; ++k)
        s = (s << 1) | F_pad(i + bit_seq[k][0], j + bit_seq[k][1]);
      G(i, j) = s;
    }
  return G;
}
