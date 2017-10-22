#include <cstdio>
#include <cmath>

int main(int argc, char *argv[]) {
  while (true) {
    double x[3];
    printf("Input initial x1 x2 x3\n");
    scanf("%lf %lf %lf", &x[0], &x[1], &x[2]);
    int iter = 0;
    while (true) {
      double J[3][3];
      double y[3], r[3];
      J[0][0] = (-x[2]) / ((2 * x[0]) * (2 + x[2]));
      J[0][1] = 1 / (2 * x[0]);
      J[0][2] = -J[0][0];
      J[1][0] = 1 / (2 * x[1]);
      J[1][1] = -J[1][0];
      J[1][2] = 0;
      J[2][0] = 1 / (4 + 2 * x[2]);
      J[2][1] = 0;
      J[2][2] = -J[2][0];
      r[0] = x[0]*x[0] + x[1]*x[1] + x[2]*x[2] - 1;
      r[1] = x[0]*x[0] + x[2]*x[2] - 0.25;
      r[2] = x[0]*x[0] + x[1]*x[1] - 4 * x[2];

      double err = 0;
      for (int i = 0; i < 3; ++i) {
        double d = 0;
        for (int j = 0; j < 3; ++j) {
          d += J[i][j] * r[j];
        }
        y[i] = x[i] - d;
        err += d*d;
      }
      for (int i = 0; i < 3; ++i) {
        x[i] = y[i];
      }
      ++iter;
      printf("Iter #%d\n", iter);
      printf("x1, x2, x3 = %.10e %.10e %.10e\nf1, f2, f3 = %.10e %.10e %.10e\n", x[0], x[1], x[2], r[0], r[1], r[1]);
      err = sqrt(err);
      if (err < 1e-7 || iter >= 50) break;
    }
  }
  return 0;
}
