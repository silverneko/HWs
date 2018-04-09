#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <queue>
#include <tuple>

#include "read_raw.h"

#define N 512

using namespace std;

float clip(float v, float l, float r) {
  return min(max(l, v), r);
}

void pad(void *I, int ni, int nw, void *O);
void filter(void *I, int ni, void *K, int nk, void *O);

// Usage: ./problem1b [infile] [outfile]
int main (int argc, char *argv[]) {
  assert(argc > 2);
  const char *infile = argv[1];
  string outfile(argv[2]);

  unsigned char data_raw[N][N];
  read_raw(infile, N*N, data_raw);

  float data_in[N][N];
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      data_in[i][j] = data_raw[i][j] / 255.0;
    }
  }

  float rot = 31 * M_PI / 180;
  float mag = (1.0 / 255) * 25;
  //float step = (M_PI * 2) * 0.06;
  float step = 0.39;
  float phase = M_PI / 4 * 0;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      double x = j * cos(rot) - (-i) * sin(rot);
      data_in[i][j] -= mag * sin(step * x + phase);
      data_in[i][j] = clip(data_in[i][j], 0, 1);
    }
  }

  unsigned char out[N][N];
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      out[i][j] = data_in[i][j] * 255;
  save_raw(outfile + ".denoise.raw", N*N, out);

  float sobelMask[2][3][3] = {
    {
      {-1, 0, 1},
      {-2, 0, 2},
      {-1, 0, 1}
    },
    {
      {1, 2, 1},
      {0, 0, 0},
      {-1, -2, -1}
    }
  };
  float gaussian5[5][5] = {
    {2, 4, 5, 4, 2},
    {4, 9, 12, 9, 4},
    {5, 12, 15, 12, 5},
    {4, 9, 12, 9, 4},
    {2, 4, 5, 4, 2}
  };

  for (int i = 0; i < 18; ++i)
    ((float*)sobelMask)[i] /= 4;

  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
      gaussian5[i][j] /= 159;

  float data_p[N+4][N+4], data_smooth[N][N];
  pad(data_in, N, 2, data_p);
  filter(data_p, N+4, gaussian5, 5, data_smooth);

  {
    float data_smooth_p[N+2][N+2], g[2][N][N];
    float mag[N][N], r[N][N], mag2[N][N];
    unsigned char out[N][N];
    // step 1: smooth
    // pad(data_in, N, 2, data_p);
    // filter(data_p, N+4, gaussian5, 5, data_smooth);
    // step 2: first order gradient magnitude and orientation
    pad(data_smooth, N, 1, data_smooth_p);
    filter(data_smooth_p, N+2, sobelMask[0], 3, g[0]);
    filter(data_smooth_p, N+2, sobelMask[1], 3, g[1]);
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j) {
        float x = g[0][i][j], y = g[1][i][j];
        r[i][j] = atan2(y, x);
        mag[i][j] = sqrt(x*x + y*y);
      }
    // step 3: non-maximal suppression
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        float th = r[i][j];
        int angle = 0;
        if (th < 0)
          th += M_PI;
        th = th / M_PI * 180;
        if (th < 22.5)
          angle = 0;
        else if (th < 67.5)
          angle = 1;
        else if (th < 112.5)
          angle = 2;
        else if (th < 157.5)
          angle = 3;
        else
          angle = 0;
        int d[4][2] = {
          {0, 1},
          {-1, 1},
          {1, 0},
          {1, 1}
        };
        int dx = d[angle][0], dy = d[angle][1];
        float max_neighbor = 0;
        if (i+dx < 0 || i+dx >= N || j+dy < 0 || j+dy >= N) {
          // do nothing
        } else {
          max_neighbor = mag[i+dx][j+dy];
        }
        if (i-dx < 0 || i-dx >= N || j-dy < 0 || j-dy >= N) {
          // do nothing
        } else {
          max_neighbor = max(mag[i-dx][j-dy], max_neighbor);
        }
        if (mag[i][j] > max_neighbor) {
          mag2[i][j] = mag[i][j];
        } else {
          mag2[i][j] = 0;
        }

      }
    }
    // step 4: hysteretic thresholding
    float thres_h = 0.11, thres_l = 0.05;
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        if (mag2[i][j] >= thres_h)
          out[i][j] = 255;
        else if (mag2[i][j] >= thres_l)
          out[i][j] = 128;
        else
          out[i][j] = 0;
      }
    }
    // step 5: connect edges
    queue<tuple<int, int>> bfs_queue;
    bool visited[N][N] = {false};
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        if (out[i][j] == 255) {
          visited[i][j] = true;
          bfs_queue.push(make_tuple(i, j));
        } else if (out[i][j] == 0) {
          visited[i][j] = true;
        }
    while (!bfs_queue.empty()) {
      int x, y;
      tie(x, y) = bfs_queue.front();
      bfs_queue.pop();
      for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy) {
          int nx = x+dx, ny = y+dy;
          if (nx < 0 || nx >= N || ny < 0 || ny >= N) continue;
          if (visited[nx][ny]) continue;
          visited[nx][ny] = true;
          out[nx][ny] = 255;
          bfs_queue.push(make_tuple(nx, ny));
        }
    }
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        if (!visited[i][j])
          out[i][j] = 0;

    save_raw(outfile + ".raw", N*N, out);
  }


  return 0;
}

#define F(x, y) (((float*)I)[ni * (x) + (y)])
#define G(x, y) (((float*)O)[no * (x) + (y)])
#define K(x, y) (((float*)kernel)[nk * (x) + (y)])

void pad(void *I, int ni, int nw, void *O) {
  int no = ni + nw * 2;
  for (int i = 0; i < ni; ++i) {
    for (int j = 0; j < ni; ++j) {
      G(i+nw, j+nw) = F(i, j);
    }
  }
  for (int i = 0; i < nw; ++i) {
    for (int j = 0; j < ni; ++j) {
      G(i, j+nw) = G(nw, j+nw);
      G(nw + ni + i, j+nw) = G(nw+ni-1, j+nw);
      G(j+nw, i) = G(j+nw, nw);
      G(j+nw, nw + ni + i) = G(j+nw, nw+ni-1);
    }
  }
  for (int i = 0; i < nw; ++i) {
    for (int j = 0; j < nw; ++j) {
      G(i, j) = G(nw, nw);
      G(nw+ni+i, j) = G(nw+ni-1, nw);
      G(i, nw+ni+j) = G(nw, nw+ni-1);
      G(nw+ni+i, nw+ni+j) = G(nw+ni-1, nw+ni-1);
    }
  }
}

void filter(void *I, int ni, void *kernel, int nk, void *O) {
  int no = ni-(nk-1);
  for (int i = 0; i < no; ++i) {
    for (int j = 0; j < no; ++j) {
      double accu = 0;
      for (int ik = 0; ik < nk; ++ik) {
        for (int jk = 0; jk < nk; ++jk) {
          accu += K(ik, jk) * F(i+ik, j+jk);
        }
      }
      G(i, j) = accu;
    }
  }
}
