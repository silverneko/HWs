#include <cstdio>
#include <cstdlib>

#include "read_raw.h"

#define N 256

int main(int argc, char *argv[]) {
  unsigned char data_in[3][N][N];
  read_raw("raw/sample1.raw", 3*N*N, data_in);

  unsigned char data_out[N][N];
  // Convert to grayscale
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      unsigned int R = data_in[0][i][j];
      unsigned int G = data_in[1][i][j];
      unsigned int B = data_in[2][i][j];
      data_out[i][j] = 0.2126 * R + 0.7152 * G + 0.0722 * B;
    }
  }

  // Flip along the diagnal
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < i; ++j) {
      unsigned char tmp = data_out[i][j];
      data_out[i][j] = data_out[j][i];
      data_out[j][i] = tmp;
    }
  }

  save_raw("raw/warm_up.out.raw", N*N, data_out);

  return 0;
}
