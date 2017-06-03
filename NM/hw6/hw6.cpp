#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;

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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s [mat file]\n", argv[0]);
    return 1;
  }
  char *matfile = argv[1];

  ifstream fin(matfile);
  string buffer;
  int M, N, NZ;
  while (getline(fin, buffer)) {
    if (buffer[0] != '%') {
      sscanf(buffer.c_str(), "%d %d %d", &M, &N, &NZ);
      break;
    }
  }
  double diag[100000] = {0};
  vector<pair<int, double>> rows[100000];
  for (int i = 0; i < NZ; ++i) {
    int r, c;
    double a;
    fin >> r >> c >> a;
    if (r == c) {
      diag[r] = a * 1e1;
    } else {
      rows[r].emplace_back(c, a);
      rows[c].emplace_back(r, a);
    }
  }
  fin.close();

  // Do diagonal scaling
  double b[100000];
  for (int i = 1; i <= M; ++i) {
    assert(diag[i] > 0);
    b[i] = 1 / sqrt(diag[i]);
    diag[i] = 1;
  }
  CSRMat<double> A(NZ*2, M);
  int idx = 0;
  A.rowIndex[0] = A.rowIndex[1] = 0;
  for (int i = 1; i <= M; ++i) {
    sort(rows[i].begin(), rows[i].end());
    for (int j = 0; j < rows[i].size(); ++j) {
      int c = rows[i][j].first;
      A.columns[idx] = c;
      A.values[idx] = rows[i][j].second * b[i] * b[c];
      idx++;
    }
    A.rowIndex[i+1] = idx;
  }
  assert(idx == (NZ - M) * 2);

  double b0 = 0;
  for (int i = 1; i <= N; ++i)
    b0 += b[i] * b[i];
  b0 = sqrt(b0);

  // Jacobi
  cout << "===========================\n";
  cout << "Jacobi\n";
  {
    double _x[2][100000];
    double *x = _x[0], *y = _x[1];
    for (int i = 1; i <= N; ++i) {
      x[i] = 0;
    }

    int iter = 1;
    while (iter <= 50) {
      for (int i = 1; i <= M; ++i) {
        int rb = A.rowIndex[i], re = A.rowIndex[i+1];
        double t = b[i];
        for (int j = rb; j < re; ++j) {
          int c = A.columns[j];
          double a = A.values[j];
          t -= a * x[c];
        }
        y[i] = t / diag[i];
      }
      swap(x, y);
      double error = 0;
      for (int i = 1; i <= M; ++i) {
        int rb = A.rowIndex[i], re = A.rowIndex[i+1];
        double t = b[i];
        for (int j = rb; j < re; ++j) {
          int c = A.columns[j];
          double a = A.values[j];
          t -= a * x[c];
        }
        t -= diag[i] * x[i];
        error += t * t;
      }
      error = sqrt(error);
      if (error / b0 < 1e-20) {
        break;
      }
      printf("Iter %d - Error %.10e\n", iter, error);
      ++iter;
    }
  }

  // Gauss-Seidel
  cout << "===========================\n";
  cout << "Gauss-Seidel\n";
  {
    double x[100000];
    for (int i = 1; i <= N; ++i) {
      x[i] = 0;
    }

    int iter = 1;
    while (iter <= 50) {
      for (int i = 1; i <= M; ++i) {
        int rb = A.rowIndex[i], re = A.rowIndex[i+1];
        double t = b[i];
        for (int j = rb; j < re; ++j) {
          int c = A.columns[j];
          double a = A.values[j];
          t -= a * x[c];
        }
        x[i] = t / diag[i];
      }

      double error = 0;
      for (int i = 1; i <= M; ++i) {
        int rb = A.rowIndex[i], re = A.rowIndex[i+1];
        double t = b[i];
        for (int j = rb; j < re; ++j) {
          int c = A.columns[j];
          double a = A.values[j];
          t -= a * x[c];
        }
        t -= diag[i] * x[i];
        error += t * t;
      }
      error = sqrt(error);
      if (error / b0 < 1e-20) {
        break;
      }
      printf("Iter %d - Error %.10e\n", iter, error);
      ++iter;
    }
  }

  // CG
  cout << "===========================\n";
  cout << "Conjugate gradient\n";
  {
    int iter = 1;
    double x[100000] = {0};
    double r[100000], p[100000], w[100000];
    double r0 = 0, r1;
    for (int i = 1; i <= N; ++i) {
      r[i] = b[i];
      r0 += r[i] * r[i];
    }
    while (iter <= 50 && sqrt(r0) / b0 > 1e-20) {
      if (iter == 1) {
        for (int i = 1; i <= N; ++i)
          p[i] = r[i];
      } else {
        double beta = r0 / r1;
        for (int i = 1; i <= N; ++i)
          p[i] = r[i] + beta * p[i];
      }
      // w = Ap
      for (int i = 1; i <= M; ++i) {
        int rb = A.rowIndex[i], re = A.rowIndex[i+1];
        double t = 0;
        for (int j = rb; j < re; ++j) {
          int c = A.columns[j];
          double a = A.values[j];
          t += a * p[c];
        }
        t += diag[i] * p[i];
        w[i] = t;
      }
      // alpha = r0 / p'w
      double alpha = 0;
      for (int i = 1; i <= N; ++i)
        alpha += p[i] * w[i];
      alpha = r0 / alpha;
      // x = x + alpha p
      for (int i = 1; i <= N; ++i)
        x[i] += alpha * p[i];
      // r = r - alpha w
      for (int i = 1; i <= N; ++i)
        r[i] -= alpha * w[i];
      // r1 = r0; r0 = |r|^2
      r1 = r0;
      r0 = 0;
      for (int i = 1; i <= N; ++i)
        r0 += r[i] * r[i];

      double error = 0;
      for (int i = 1; i <= M; ++i) {
        int rb = A.rowIndex[i], re = A.rowIndex[i+1];
        double t = b[i];
        for (int j = rb; j < re; ++j) {
          int c = A.columns[j];
          double a = A.values[j];
          t -= a * x[c];
        }
        t -= diag[i] * x[i];
        error += t * t;
      }
      error = sqrt(error);
      printf("Iter %d - Error %.10e\n", iter, error);
      ++iter;
    }
  }

  return 0;
}
