#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <queue>
#include <tuple>
#include <vector>
#include <queue>

#include "read_raw.h"

#define N 512

using namespace std;

Array2d<bool> shrink(Array2d<bool> mask) {
  Array2d<bool> nmask(mask.dim[0], mask.dim[1]);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      nmask(i, j) = mask(i, j);
      if (!mask(i, j))
        continue;

      int d[8][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
      for (int k = 0; k < 8; ++k) {
        int x = i, y = j;
        int dx = d[k][0], dy = d[k][1];
        if (x+dx < 0 || x+dx >= mask.dim[0] || y+dy < 0 || y+dy >= mask.dim[1])
          continue;
        if (!mask(x+dx, y+dy)) {
          nmask(x, y) = false;
          break;
        }
      }
    }
  return nmask;
}

// Usage: ./problem2b [infile] [infile K] [outfile]
int main (int argc, char *argv[]) {
  assert(argc > 3);
  const char *infile = argv[1];
  const char *infile2 = argv[2];
  string outfile(argv[3]);

  unsigned char data_raw[N][N], data_out[N][N];
  unsigned char K_in[N][N];
  read_raw(infile, N*N, data_raw);
  read_raw(infile2, N*N, K_in);

  Array2d<double> data_in(N, N);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      data_in(i, j) = data_raw[i][j] / 255.0;

  Array2d<bool> visited(N, N);
  fill_n(visited[0], N*N, false);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      if (visited(i, j))
        continue;
      visited(i, j) = true;

      int fill_color = 0;
      vector<pair<int,int>> fill_area;
      queue<pair<int,int>> bfs;
      bfs.emplace(i, j);
      fill_area.emplace_back(i, j);
      while (!bfs.empty()) {
        pair<int,int> p = bfs.front();
        bfs.pop();
        int x = p.first, y = p.second;
        int d[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        for (int k = 0; k < 4; ++k) {
          int dx = d[k][0], dy = d[k][1];
          if (x+dx < 0 || x+dx >= N || y+dy < 0 || y+dy >= N)
            continue;
          if (K_in[x][y] != K_in[x+dx][y+dy]) {
            fill_color = K_in[x+dx][y+dy];
            continue;
          }
          if (visited(x+dx, y+dy))
            continue;
          visited(x+dx, y+dy) = true;
          bfs.emplace(x+dx, y+dy);
          fill_area.emplace_back(x+dx, y+dy);
        }
      }
      if (fill_area.size() < 10000)
        for (auto p : fill_area)
          K_in[p.first][p.second] = fill_color;
    }

  int color[256], cc = 0;
  fill_n(color, 256, -1);
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      if (color[ K_in[i][j]] == -1)
        color[K_in[i][j]] = cc++;
      K_in[i][j] = color[K_in[i][j]];
    }

  Array2d<double> F(N, N);
  Array2d<double> T[10];
  assert(cc < 10);
  copy_n(data_in[0], N*N, F[0]);
  for (int k = 0; k < cc; ++k) {
    Array2d<bool> mask(N, N);
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        mask(i, j) = K_in[i][j] == k;
    // shrink the mask, discard texture boundary
    for (int i = 0; i < 10; ++i) {
      mask = shrink(mask);
    }

    // use autocorrelation to find repeating pattern
    // first shift signal to zero mean
    double mean = 0;
    {
      int n = 0;
      for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
          if (mask(i, j)) {
            mean += F(i, j);
            ++n;
          }
      mean /= n;
    }
    Array2d<double> ac(100, 100);
    {
      double maxv = 0, minv = 1;
      for (int di = 0; di < ac.dim[0]; ++di)
        for (int dj = 0; dj < ac.dim[1]; ++dj) {
          double accu = 0.0;
          double n = 0;
          for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j) {
              if (i+di < 0 || i+di >= N || j+dj < 0 || j+dj >= N)
                continue;
              if (mask(i, j) && mask(i+di, j+dj)) {
                accu += (F(i, j) - mean) * (F(i+di, j+dj) - mean);
                n += (F(i, j) - mean) * (F(i, j) - mean);
              }
            }
          accu /= n;
          ac(di, dj) = accu;
          maxv = max(maxv, accu);
          minv = min(minv, accu);
        }
      for (int i = 0; i < ac.dim[0]; ++i)
        for (int j = 0; j < ac.dim[1]; ++j)
          ac(i, j) = (ac(i, j) - minv) / (maxv - minv);
    }

    for (int i = 0; i < ac.dim[0]; ++i)
      for (int j = 0; j < ac.dim[1]; ++j)
        ((unsigned char*)data_out)[i*ac.dim[1]+j] = ac(i, j) * 255;
    char buf[256];
    snprintf(buf, 256, "E.ac.%d.raw", k);
    save_raw(buf, ac.dim[0]*ac.dim[1], data_out);

    // calculate ac2
    Array2d<double> ac2(100, 100);
    {
      double maxv = 0, minv = 1;
      for (int di = 0; di < ac2.dim[0]; ++di)
        for (int dj = 0; dj < ac2.dim[1]; ++dj) {
          double accu = 0.0;
          double n = 0;
          for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j) {
              if (i-di < 0 || i-di >= N || j+dj < 0 || j+dj >= N)
                continue;
              if (mask(i, j) && mask(i-di, j+dj)) {
                accu += (F(i, j) - mean) * (F(i-di, j+dj) - mean);
                n += (F(i, j) - mean) * (F(i, j) - mean);
              }
            }
          accu /= n;
          ac2(di, dj) = accu;
          maxv = max(maxv, accu);
          minv = min(minv, accu);
        }
      for (int i = 0; i < ac.dim[0]; ++i)
        for (int j = 0; j < ac.dim[1]; ++j)
          ac2(i, j) = (ac2(i, j) - minv) / (maxv - minv);
    }

    // find local maximum of autocorrelation, the "peak" ac map
    int dx, dy, dx2, dy2;
    {
      Array2d<bool> local_max(ac.dim[0], ac.dim[1]);
      for (int i = 0; i < ac.dim[0]; ++i)
        for (int j = 0; j < ac.dim[1]; ++j) {
          bool is_local_max = true;
          for (int ii = max(0, i-20); ii < min(N, i+1+20); ++ii)
            for (int jj = max(0, j-20); jj < min(N, j+1+20); ++jj)
              if (ac(i, j) < ac(ii, jj)) {
                is_local_max = false;
                goto not_local_maximal_label;
              }
  not_local_maximal_label:
          local_max(i, j) = is_local_max;
        }

      for (int i = 0; i < ac.dim[0]; ++i)
        for (int j = 0; j < ac.dim[1]; ++j)
          ((unsigned char*)data_out)[i*ac.dim[1]+j] = local_max(i, j) * ac(i, j) * 255;
      snprintf(buf, 256, "E.ac.peak.%d.raw", k);
      save_raw(buf, ac.dim[0] * ac.dim[1], data_out);

      // find the pattern cycle
      vector<pair<double, pair<int, int>>> ac_rank;
      for (int i = 0; i < ac.dim[0]; ++i)
        for (int j = 0; j < ac.dim[1]; ++j)
          if (local_max(i, j))
            ac_rank.push_back(make_pair(ac(i, j), make_pair(i, j)));
      sort(ac_rank.begin(), ac_rank.end());
      reverse(ac_rank.begin(), ac_rank.end());
      dx = ac_rank[1].second.first;
      dy = ac_rank[1].second.second;
    }
    {
      Array2d<bool> local_max(ac2.dim[0], ac2.dim[1]);
      for (int i = 0; i < ac2.dim[0]; ++i)
        for (int j = 0; j < ac2.dim[1]; ++j) {
          bool is_local_max = true;
          for (int ii = max(0, i-20); ii < min(N, i+1+20); ++ii)
            for (int jj = max(0, j-20); jj < min(N, j+1+20); ++jj)
              if (ac2(i, j) < ac2(ii, jj)) {
                is_local_max = false;
                goto not_local_maximal_label2;
              }
  not_local_maximal_label2:
          local_max(i, j) = is_local_max;
        }
      // find the pattern cycle
      vector<pair<double, pair<int, int>>> ac_rank;
      for (int i = 0; i < ac2.dim[0]; ++i)
        for (int j = 0; j < ac2.dim[1]; ++j)
          if (local_max(i, j))
            ac_rank.push_back(make_pair(ac2(i, j), make_pair(i, j)));
      sort(ac_rank.begin(), ac_rank.end());
      reverse(ac_rank.begin(), ac_rank.end());
      dx2 = - ac_rank[1].second.first;
      dy2 = ac_rank[1].second.second;
    }

    // synthesize the whole texture by repeating pattern cycle
    T[k] = Array2d<double>(N, N);
    Array2d<bool> visited(N, N);
    fill_n(T[k][0], N*N, 0);
    fill_n(visited[0], N*N, false);
    queue<pair<int,int>> bfs;
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        if (mask(i, j)) {
          T[k](i, j) = F(i, j);
          bfs.emplace(i, j);
          visited(i, j) = true;
        }
    while (!bfs.empty()) {
      pair<int,int> p = bfs.front();
      bfs.pop();
      int x = p.first, y = p.second;
      int d[4][2] = {{dx, dy}, {-dx, -dy}, {dx2, dy2}, {-dx2, -dy2}};
      for (int l = 0; l < 4; ++l) {
        int nx = x+d[l][0], ny = y+d[l][1];
        if (nx < 0 || nx >= N || ny < 0 || ny >= N)
          continue;
        if (visited(nx, ny))
          continue;
        visited(nx, ny) = true;
        T[k](nx, ny) = T[k](x, y);
        bfs.emplace(nx, ny);
      }
    }
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        data_out[i][j] = T[k](i, j) * 255;
    snprintf(buf, 256, "T.%d.raw", k);
    save_raw(buf, N*N, data_out);
  }

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      int t = K_in[i][j] - 1;
      if (t == -1)
        t = cc-1;
      data_out[i][j] = T[t](i, j) * 255;
    }

  save_raw(outfile, N*N, data_out);

  return 0;
}
