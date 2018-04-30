#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <queue>
#include <tuple>
#include <random>

#include "read_raw.h"

#define N 512

using namespace std;

Array2d<double> energy_computation1(Array2d<double> I) {
  const int w = 13;
  const int hw = (w-1)/2;
  Array2d<double> O(I.dim[0], I.dim[1]);
  for (int i = 0; i < O.dim[0]; ++i)
    for (int j = 0; j < O.dim[1]; ++j) {
      double accu = 0;
      int n = 0;
      for (int ii = max(0, i-hw); ii < min(I.dim[0], i+1+hw); ++ii)
        for (int jj = max(0, j-hw); jj < min(I.dim[1], j+1+hw); ++jj) {
          accu += I(ii, jj) * I(ii, jj);
          ++n;
        }
      O(i, j) = sqrt(accu / n);
    }
  return O;
}

Array2d<double> energy_computation2(Array2d<double> I) {
  const int w = 13;
  const int hw = (w-1)/2;
  Array2d<double> O(I.dim[0], I.dim[1]);
  for (int i = 0; i < O.dim[0]; ++i)
    for (int j = 0; j < O.dim[1]; ++j) {
      double accu = 0;
      int n = 0;
      for (int ii = max(0, i-hw); ii < min(I.dim[0], i+1+hw); ++ii)
        for (int jj = max(0, j-hw); jj < min(I.dim[1], j+1+hw); ++jj) {
          accu += I(ii, jj);
          ++n;
        }
      O(i, j) = accu / n;
    }
  return O;
}

// Usage: ./problem2a [infile] [outfile]
int main (int argc, char *argv[]) {
  assert(argc > 2);
  const char *infile = argv[1];
  string outfile(argv[2]);

  unsigned char data_raw[N][N], data_out[N][N];
  read_raw(infile, N*N, data_raw);

  Array2d<double> data_in(N, N);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      data_in(i, j) = data_raw[i][j] / 255.0;

  double L5[5] = {1, 4, 6, 4, 1};
  double S5[5] = {1, 0, -2, 0, 1};
  double R5[5] = {1, -4, 6, -4, 1};
  double E5[5] = {-1, -2, 0, 2, 1};
  double W5[5] = {-1, 2, 0, -2, 1};
  double *mask1d[5] = {L5, S5, R5, E5, W5};
  const int n_laws_channel = 25;
  Array2d<double> mask2d[n_laws_channel];
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j) {
      int k = i*5+j;
      mask2d[k] = Array2d<double>(5, 5);
      for (int ii = 0; ii < 5; ++ii)
        for (int jj = 0; jj < 5; ++jj)
          mask2d[k][ii][jj] = mask1d[i][ii] * mask1d[j][jj];
    }

  Array2d<double> pad_F = pad(data_in, 2);
  Array2d<double> feature_map[n_laws_channel];
  for (int k = 0; k < n_laws_channel; ++k)
    feature_map[k] = filter(pad_F, mask2d[k]);

  const int max_feature = n_laws_channel * 2;
  Array2d<double> feature_set[max_feature];
  for (int k = 0; k < n_laws_channel; ++k) {
    feature_set[2*k] = energy_computation1(feature_map[k]);
    feature_set[2*k+1] = energy_computation2(feature_map[k]);
  }

  // z-normalization
  double mean[max_feature], stddev[max_feature];
  for (int k = 0; k < max_feature; ++k) {
    double sum = 0, sigma = 0;
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        sum += feature_set[k](i, j);
    mean[k] = sum / (N*N);
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        sigma += pow(feature_set[k](i, j) - mean[k], 2);
    stddev[k] = sqrt(sigma / (N*N));
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        feature_set[k](i, j) = (feature_set[k](i, j) - mean[k]) / stddev[k];
  }

  const int n_feature = max_feature;
  double (*feature_vector)[N][max_feature] = new double[N][N][max_feature];
  for (int k = 0; k < n_feature; ++k)
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        feature_vector[i][j][k] = feature_set[k][i][j];

  // K-means
  const int n_cluster = 3;
  double centroid[n_cluster][max_feature];
  int cluster_id[N][N] = {0};
  // Initialize: Randomly select centroid
  std::mt19937 rng(0x7d34ae54);
  std::uniform_int_distribution<int> dist(0, N-1);
  bool selected[N] = {0};
  for (int i = 0; i < n_cluster; ++i) {
    int x = dist(rng), y = dist(rng);
    while (selected[x])
      x = dist(rng);
    while (selected[y])
      y = dist(rng);
    selected[x] = selected[y] = true;
    for (int k = 0; k < n_feature; ++k)
      centroid[i][k] = feature_vector[x][y][k];
  }

  // Repeat until converge
  int iterate_count = 0;
  while (true) {
    bool converged = true;
    // Calculate new cluster by nearest neighbor
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j) {
        int cid = 0;
        double dist = 0;
        for (int l = 0; l < n_cluster; ++l) {
          double accu = 0;
          for (int k = 0; k < n_feature; ++k)
            accu += pow(feature_vector[i][j][k] - centroid[l][k], 2);
          if (l == 0 || dist > accu) {
            dist = accu;
            cid = l;
          }
        }
        if (cluster_id[i][j] != cid) {
          cluster_id[i][j] = cid;
          converged = false;
        }
      }
    if (converged)
      break;
    // Calculate new cluster centroid
    for (int c = 0; c < n_cluster; ++c) {
      double nc[max_feature] = {0};
      int n = 0;
      for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
          if (cluster_id[i][j] == c) {
            for (int k = 0; k < n_feature; ++k)
              nc[k] += feature_vector[i][j][k];
            ++n;
          }
      for (int k = 0; k < n_feature; ++k)
        centroid[c][k] = nc[k] / n;
    }
    ++iterate_count;
  }
  cout << "K-means converged after " << iterate_count << " iterations.\n";

  int color[n_cluster] = {0};
  for (int i = 1; i < n_cluster; ++i)
    color[i] = 255 * i / (n_cluster-1);

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      data_out[i][j] = color[ cluster_id[i][j] ];

  save_raw(outfile, N*N, data_out);

  return 0;
}
