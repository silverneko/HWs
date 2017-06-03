#include <chrono>
#include <cstdio>
#include <random>

#include <mkl.h>

#define N 8000
double A[N][N], B[N][N], C[N][N];

int main(int argc, char *argv[]) {
  std::mt19937 randomSeed(0x5EED);
  std::uniform_real_distribution<double> dis(-1e100, 1e100);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      A[i][j] = dis(randomSeed);
      B[i][j] = dis(randomSeed);
    }
  }

  double alpha = 1, beta = 0;
  std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, N, N, N,
              alpha, (double*)A, N, (double*)B, N, beta, (double*)C, N);

  std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_elapsed = t1 - t0;
  printf("Time elapsed: %.6f seconds\n", time_elapsed);
  return 0;
}
