#include <cassert>
#include <chrono>
#include <cstdio>
#include <random>

#include <mkl.h>

template <typename T>
struct CSRMat {
public:
  CSRMat() : n(0), values(nullptr), columns(nullptr), rowIndex(nullptr) {}

  CSRMat(int sizen, int sizem) : n(sizen), m(sizem) {
    values = new T[n+1];
    columns = new int[n+1];
    rowIndex = new int[m+2];
  }

  ~CSRMat() {
    delete[] values;
    delete[] columns;
    delete[] rowIndex;
  }

  // Number of elements
  int n;
  // Number of rows
  int m;
  T *values;
  int *columns;
  int *rowIndex;
};

std::mt19937 Seed(0x5EED);

void test(int m, double sparsity) {
  int sizen = (uint64_t)m * m * sparsity;
  CSRMat<double> A(sizen, m);
  CSRMat<double> B(sizen, m);
  CSRMat<double> C(m*m, m);
  auto populate = [](CSRMat<double> &A, int sizen) {
    std::uniform_real_distribution<double> dis(-1e100, 1e100);
    std::uniform_real_distribution<double> roulette(0, 1);
    int idx = 0;
    int m = A.m;
    uint64_t left = (uint64_t)m * m;
    int need = sizen;
    for (int i = 0; i < m; ++i) {
      A.rowIndex[i] = idx+1;
      for (int j = 0; j < m && need > 0; ++j) {
        double p = roulette(Seed);
        if (p * left <= need) {
          A.values[idx] = dis(Seed);
          A.columns[idx] = j+1;
          ++idx;
          --need;
        }
        --left;
      }
    }
    A.rowIndex[m] = idx+1;
    assert(idx == A.n);
  };
  populate(A, sizen);
  populate(B, sizen);

  printf("Mat size: %d x %d; Sparsity: %.6f\n", m, m, sparsity);
  std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

  const int sample_size = 20;
  for (int i = 0; i < sample_size; ++i) {
    // Compute C = A * B
    const int request = 0;
    const int sort = 7;
    int info = 0;
    mkl_dcsrmultcsr("N",
                    &request,
                    &sort,
                    &m,
                    &m,
                    &m,
                    A.values,
                    A.columns,
                    A.rowIndex,
                    B.values,
                    B.columns,
                    B.rowIndex,
                    C.values,
                    C.columns,
                    C.rowIndex,
                    &C.n,
                    &info
                    );
    assert(info == 0);
  }

  std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_elapsed = t1 - t0;
  printf("Average elapse time: %.6f seconds\n", time_elapsed.count() / sample_size);
#if 0
  auto print_mat = [](const CSRMat<double> &A) {
    int m = A.m;
    for (int i = 0; i < m; ++i) {
      int col = 1;
      for (int j = A.rowIndex[i]; j < A.rowIndex[i+1]; ++j) {
        while (col < A.columns[j-1]) {
          ++col;
          printf("0.00 ");
        }
        printf("%.2f ", A.values[j-1]);
        ++col;
      }
      while (col <= A.m) {
        ++col;
        printf("0.00 ");
      }
      printf("\n");
    }
  };
  print_mat(A);
  printf("\n");
  print_mat(B);
  printf("\n");
  print_mat(C);
  printf("\n");
#endif
}

int main(int argc, char *argv[]) {
  test(1000, 0.5);
  test(1000, 0.1);
  test(10000, 0.01);
  test(10000, 0.001);
  test(10000, 0.0001);
  return 0;
}
