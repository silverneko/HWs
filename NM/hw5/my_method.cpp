#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <random>

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

void my_dcsrmm(const CSRMat<double> &A, const CSRMat<double> &B,
	       CSRMat<double> &C) {
  int m = A.m;
  bool *col_visited = new bool[m+1];
  double *row_cache = new double[m+1];
  memset(col_visited, 0, sizeof(bool) * (m+1));
  memset(row_cache, 0, sizeof(double) * (m+1));
  int cidx = 0;
  for (int i = 0; i < m; ++i) {
    const int b = A.rowIndex[i]-1, e = A.rowIndex[i+1]-1;
    int *col_idx = &C.columns[cidx];
    int col_idxn = 0;
    for (int j = b; j < e; ++j) {
      const double aij = A.values[j];
      const int c = A.columns[j]-1;
      const int bb = B.rowIndex[c]-1, be = B.rowIndex[c+1]-1;
      for (int k = bb; k < be; ++k) {
	const double bjk = B.values[k];
	const int bc = B.columns[k];
	if (!col_visited[bc]) {
	  col_visited[bc] = true;
	  col_idx[col_idxn++] = bc;
	}
	row_cache[bc] += aij * bjk;
      }
    }
    C.rowIndex[i] = cidx+1;
    // std::sort(col_idx, col_idx + col_idxn);
    for (int j = 0; j < col_idxn; ++j) {
      const int col = col_idx[j];
      C.values[cidx++] = row_cache[col];
      row_cache[col] = 0.0;
      col_visited[col] = false;
    }
  }
  C.rowIndex[m] = cidx+1;
  C.n = cidx;
  delete[] col_visited;
  delete[] row_cache;
}

std::mt19937 Seed(0x5EED);

void test(int m, double sparsity) {
  int sizen = (uint64_t)m * m * sparsity;
  CSRMat<double> A(sizen, m);
  CSRMat<double> B(sizen, m);
  CSRMat<double> C(m*m, m);
  auto populate = [](CSRMat<double> &A, int sizen) {
    // std::uniform_real_distribution<double> dis(-1e100, 1e100);
    std::uniform_real_distribution<double> dis(0, 10);
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
    A.rowIndex[m] = A.n+1;
    assert(idx == A.n);
  };
  populate(A, sizen);
  populate(B, sizen);

  printf("Mat size: %d x %d; Sparsity: %.6f\n", m, m, sparsity);
  std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
  const int sample_size = 20;
  for (int i = 0; i < sample_size; ++i) {
    // Compute C = A * B
    my_dcsrmm(A, B, C);
  }

  std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_elapsed = t1 - t0;
  printf("Average elapse time: %.6f seconds\n", time_elapsed.count() / sample_size);
}

int main(int argc, char *argv[]) {
  test(1000, 0.5);
  test(1000, 0.1);
  test(10000, 0.01);
  test(10000, 0.001);
  test(10000, 0.0001);
  return 0;
}
