#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <queue>
#include <tuple>
#include <vector>

#include "read_raw.h"

using namespace std;

template<typename T>
Array2d<T> medianFilter(Array2d<T> F, int nw);
int euler_number(Array2d<int> F);
vector<double> moment_features(Array2d<int> F);
tuple<vector<double>, vector<double>> proj_histogram(Array2d<int> F);
double l1(const vector<double> &v1, const vector<double> &v2);
double l2(const vector<double> &v1, const vector<double> &v2);
double linf(const vector<double> &v1, const vector<double> &v2);

// Usage: ./problem1 [model] [infile]
int main (int argc, char *argv[]) {
  assert(argc > 2);
  const char *infile = argv[2];

  const int W = 390, H = 125;
  unsigned char data_raw[H][W];
  read_raw(infile, H*W, data_raw);

  vector<char> y_label;
  vector<Array2d<int>> y_pat;
  {
    FILE *fp = fopen(argv[1], "r");
    char buf[2048];
    char l;
    int h, w;
    while (fscanf(fp, "%c,%d,%d,%s\n", &l, &h, &w, buf) != EOF) {
      y_label.push_back(l);
      Array2d<int> pat(h, w);
      for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
          pat(i, j) = buf[i * w + j] == '1';
      y_pat.push_back(pat);
    }
    fclose(fp);
  }

  // data augmentation
  for (int k = 0, nk = y_label.size(); k < nk; ++k) {
    if (y_label[k] == '!') {
      y_label.push_back('i');
      Array2d<int> pat(y_pat[k].dim[0], y_pat[k].dim[1]);
      for (int i = 0; i < pat.dim[0]; ++i)
        for (int j = 0; j < pat.dim[1]; ++j)
          pat(i, j) = y_pat[k](pat.dim[0]-1-i, j);
      y_pat.push_back(pat);
    }
  }

  auto extract_features = [](Array2d<int> F) {
    int E = euler_number(F);
    vector<double> v1 = moment_features(F);
    int lb = F.dim[1], rb = 0;
    int ub = F.dim[0], db = 0;
    for (int i = 0; i < F.dim[0]; ++i)
      for (int j = 0; j < F.dim[1]; ++j)
        if (F(i, j)) {
          lb = min(lb, j);
          rb = max(rb, j+1);
          ub = min(ub, i);
          db = max(db, i+1);
        }
    v1.push_back(1.0 * (db - ub) / (rb - lb));
    vector<double> histx, histy;
    tie(histx, histy) = proj_histogram(F);
    return make_tuple(E, v1, histx, histy);
  };
  vector<int> y_E(y_pat.size());
  vector<vector<double>> y_feature(y_pat.size());
  vector<vector<double>> y_histx(y_pat.size()), y_histy(y_pat.size());
  for (int k = 0; k < y_pat.size(); ++k)
    tie(y_E[k], y_feature[k], y_histx[k], y_histy[k]) = extract_features(y_pat[k]);

  vector<double> norm_u, norm_s;
  for (int i = 0; i < y_feature[0].size(); ++i) {
    double u = 0, s = 0;
    for (int k = 0; k < y_feature.size(); ++k)
      u += y_feature[k][i];
    u /= y_feature.size();
    for (int k = 0; k < y_feature.size(); ++k)
      s += pow(y_feature[k][i] - u, 2);
    s = sqrt(s / y_feature.size());
    for (int k = 0; k < y_feature.size(); ++k)
      y_feature[k][i] = (y_feature[k][i] - u) / s;
    norm_u.push_back(u);
    norm_s.push_back(s);
  }

  /*
  for (int i = 0; i < y_feature.size(); ++i) {
    cout << y_label[i] << ' ';
    for (int j = 0; j < y_feature[i].size(); ++j)
      cout << y_feature[i][j] << ' ';
    cout << endl;
  }
  */

  Array2d<int> data_in(H, W);
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j)
      data_in(i, j) = data_raw[i][j];

  Array2d<int> G = medianFilter(data_in, 3);
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j)
      if (G(i, j) < 100)
        G(i, j) = 1;
      else
        G(i, j) = 0;

  // Segment characters by bounding box
  bool projy[W];
  fill(projy, projy + W, false);
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j)
      if (G(i, j))
        projy[j] = true;
  int pos[20][4], posn = 0;

  int state = 0;
  for (int j = 0; j < W; ++j) {
    if (state == 0) {
      if (projy[j] == true) {
        pos[posn][0] = j;
        state = 1;
      }
    } else if (state == 1) {
      if (projy[j] == false) {
        pos[posn][1] = j;
        ++posn;
        state = 0;
      }
    }
  }
  if (state == 1) {
    pos[posn][1] = W;
    ++posn;
  }
  for (int k = 0; k < posn; ++k) {
    int ub = H, lb = 0;
    for (int i = 0; i < H; ++i)
      for (int j = pos[k][0]; j < pos[k][1]; ++j)
        if (G(i, j)) {
          ub = min(ub, i);
          lb = max(lb, i+1);
        }
    pos[k][2] = ub;
    pos[k][3] = lb;
  }

  vector<char> predict;
  for (int k = 0; k < posn; ++k) {
    Array2d<int> patch(pos[k][3] - pos[k][2] + 2, pos[k][1] - pos[k][0] + 2);
    fill(patch[0], patch[0] + patch.dim[0] * patch.dim[1], 0);
    for (int i = 0; i < pos[k][3] - pos[k][2]; ++i)
      for (int j = 0; j < pos[k][1] - pos[k][0]; ++j)
        patch(i+1, j+1) = G(pos[k][2] + i, pos[k][0] + j);

    int E;
    vector<double> patch_feature, histx, histy;
    tie(E, patch_feature, histx, histy) = extract_features(patch);
    for (int i = 0; i < patch_feature.size(); ++i)
      patch_feature[i] = (patch_feature[i] - norm_u[i]) / norm_s[i];
    /*
    for (int i = 0; i < patch_feature.size(); ++i)
      cout << patch_feature[i] << ' ';
    cout << endl;
    */

    // Euler number
    int id = -1;
    double dist = 1e100;
    for (int k = 0; k < y_feature.size(); ++k) {
      if (y_E[k] != E)
        continue;
      double d = l2(histx, y_histx[k]) + l2(histy, y_histy[k]);
      if (dist > d) {
        dist = d;
        id = k;
      }
    }
    assert(id != -1);
    predict.push_back(y_label[id]);
  }


  for (int i = 0; i < predict.size(); ++i)
    cout << predict[i];
  cout << endl;

  unsigned char data_out[H][W];
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j)
      if (G(i, j) == 1)
        data_out[i][j] = 0;
      else
        data_out[i][j] = 255;
  for (int k = 0; k < posn; ++k)
    for (int i = 0; i < H; ++i) {
      data_out[i][pos[k][0]] = 127;
      data_out[i][pos[k][1]] = 127;
    }

  //save_raw("out.raw", H*W, data_out);

  return 0;
}

tuple<vector<double>, vector<double>> proj_histogram(Array2d<int> F) {
  vector<double> histx(10), histy(10);
  // compute bounding box
  int lb = F.dim[1], rb = 0;
  int ub = F.dim[0], db = 0;
  for (int i = 0; i < F.dim[0]; ++i)
    for (int j = 0; j < F.dim[1]; ++j)
      if (F(i, j)) {
        lb = min(lb, j);
        rb = max(rb, j+1);
        ub = min(ub, i);
        db = max(db, i+1);
      }
  // compute bin histogram
  for (int i = 0; ub + i < db; ++i)
    for (int j = 0; lb + j < rb; ++j) {
      int kx = floor(histx.size() * 1.0 * i / (db - ub));
      int ky = floor(histy.size() * 1.0 * j / (rb - lb));
      histx[kx] += F(ub+i, lb+j);
      histy[ky] += F(ub+i, lb+j);
    }
  for (int i = 1; i < histx.size(); ++i)
    histx[i] += histx[i-1];
  for (int i = 0; i < histx.size(); ++i)
    histx[i] /= histx.back();
  for (int i = 1; i < histy.size(); ++i)
    histy[i] += histy[i-1];
  for (int i = 0; i < histy.size(); ++i)
    histy[i] /= histy.back();
  return {histx, histy};
}

double l1(const vector<double> &v1, const vector<double> &v2) {
  double result = 0.0;
  int n = min(v1.size(), v2.size());
  for (int i = 0; i < n; ++i)
    result += fabs(v1[i] - v2[i]);
  return result / n;
}

double l2(const vector<double> &v1, const vector<double> &v2) {
  double result = 0.0;
  int n = min(v1.size(), v2.size());
  for (int i = 0; i < n; ++i)
    result += pow(v1[i] - v2[i], 2);
  return sqrt(result / n);
}

double linf(const vector<double> &v1, const vector<double> &v2) {
  double result = 0.0;
  int n = min(v1.size(), v2.size());
  for (int i = 0; i < n; ++i)
    result = max(result, fabs(v1[i] - v2[i]));
  return result;
}

vector<double> moment_features(Array2d<int> F) {
  double m00 = 0, m10 = 0, m01 = 0;
  for (int i = 0; i < F.dim[0]; ++i)
    for (int j = 0; j < F.dim[1]; ++j) {
      m00 += F(i, j);
      m10 += i * F(i, j);
      m01 += j * F(i, j);
    }
  const double cx = m10 / m00, cy = m01 / m00;
  double um[4][4] = {0}, nm[4][4] = {0};
  for (int p = 0; p < 4; ++p)
    for (int q = 0; q < 4; ++q) {
      for (int i = 0; i < F.dim[0]; ++i)
        for (int j = 0; j < F.dim[1]; ++j) {
          um[p][q] += pow(i-cx, p) * pow(j-cy, q) * F(i, j);
        }
      nm[p][q] = um[p][q] / pow(um[0][0], (p+q+2) / 2.0);
    }
  double orientation = atan(2 * um[1][1] / (um[2][0] - um[0][2]));
  double a = 1, b = -(um[2][0] + um[0][2]);
  double c = um[2][0] * um[0][2] - um[1][1] * um[1][1];
  double eigenvalue[2] = {
    (-b + sqrt(b*b - 4*a*c)) / (2 * a) / um[0][0],
    (-b - sqrt(b*b - 4*a*c)) / (2 * a) / um[0][0]
  };
  assert(eigenvalue[0] >= 0);
  assert(eigenvalue[1] >= 0);
  assert(eigenvalue[0] >= eigenvalue[1]);
  // Hu moments
  double Im[8] = {0};
  Im[1] = nm[2][0] + nm[0][2];
  Im[2] = pow(nm[2][0] - nm[0][2], 2) + 4 * pow(nm[1][1], 2);
  Im[3] = pow(nm[3][0] - 3 * nm[1][2], 2) + pow(nm[0][3] - 3 * nm[2][1], 2);
  Im[4] = pow(nm[3][0] + nm[1][2], 2) + pow(nm[0][3] + nm[2][1], 2);
  Im[5] = (nm[3][0] - 3*nm[1][2]) * (nm[3][0] + nm[1][2])
    * (pow(nm[3][0]+nm[1][2], 2) - 3*pow(nm[0][3] + nm[2][1], 2))
    + (3*nm[2][1] - nm[0][3]) * (nm[0][3] + nm[2][1])
    * (3*pow(nm[3][0]+nm[1][2], 2) - pow(nm[0][3] + nm[2][1], 2));
  Im[6] = (nm[2][0] - nm[0][2])
    * (pow(nm[3][0] + nm[1][2], 2) - pow(nm[2][1] + nm[0][3], 2))
    + 4*pow(nm[1][1], 2) * (nm[3][0] + nm[1][2]) * (nm[2][1] + nm[0][3]);
  Im[7] = (3*nm[2][1] - nm[0][3]) * (nm[3][0] + nm[1][2])
    * (pow(nm[3][0] + nm[1][2], 2) - 3*pow(nm[2][1] + nm[0][3], 2))
    + (3*nm[1][2] - nm[3][0]) * (nm[2][1] + nm[0][3])
    * (3*pow(nm[3][0] + nm[1][2], 2) - pow(nm[2][1] + nm[0][3], 2));
  vector<double> features;
  features.push_back(orientation);
  features.push_back(eigenvalue[0] / eigenvalue[1]);
  features.insert(features.end(), Im+1, Im+8);
  return features;
}

int euler_number(Array2d<int> F) {
  auto nQuad = [](Array2d<int> F, int pat[2][2]) {
    int result = 0;
    for (int i = 0; i + 1 < F.dim[0]; ++i)
      for (int j = 0; j + 1 < F.dim[1]; ++j) {
        int match = 1;
        for (int ii = 0; ii < 2; ++ii)
          for (int jj = 0; jj < 2; ++jj)
            if (F(i+ii, j+jj) != pat[ii][jj]) {
              match = 0;
              break;
            }
        result += match;
      }
    return result;
  };
  Array2d<int> H(2, 2);
  int q1 = 0, q2 = 0, q3 = 0, q4 = 0, qd = 0;
  int q1_pat[4][2][2] = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
  };
  int q2_pat[4][2][2] = {
    {1, 1, 0, 0},
    {0, 1, 0, 1},
    {0, 0, 1, 1},
    {1, 0, 1, 0}
  };
  int q3_pat[4][2][2] = {
    {1, 1, 1, 0},
    {0, 1, 1, 1},
    {1, 0, 1, 1},
    {1, 1, 0, 1}
  };
  int q4_pat[2][2] = {1, 1, 1, 1};
  int qd_pat[][2][2] = {
    {1, 0, 0, 1},
    {0, 1, 1, 0}
  };
  for (int k = 0; k < 4; ++k) {
    q1 += nQuad(F, q1_pat[k]);
    q2 += nQuad(F, q2_pat[k]);
    q3 += nQuad(F, q3_pat[k]);
  }
  q4 = nQuad(F, q4_pat);
  qd = nQuad(F, qd_pat[0]) + nQuad(F, qd_pat[1]);
  int A = (q1 + 2*q2 + 3*q3 + 4*q4 + 2*qd) / 4;
  int P = q1 + q2 + q3 + 2*qd;
  int E = (q1 - q3 - 2*qd) / 4;
  double C = 1.0 * A / (P * P);
  return E;
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
