#include <cstdio>
#include <cmath>
#include <functional>

template<typename F1, typename F2>
double newton(F1 f, F2 f_, double x0, unsigned maxIter = 10) {
  if (fabs(f_(x0)) < 1e-15) {
    printf("|f'(%.10e)| = %.10e is too small.\n", x0, fabs(f_(x0)));
    return x0;
  }
  double x = x0;
  for (int i = 0; i < maxIter; ++i) {
    if (fabs(f_(x)) < 1e-15) {
      break;
    }
    x = x - f(x) / f_(x);
    printf("#%d\t| x = %.10e | f(x) = %.10e\n", i+1, x, f(x));
  }
  return x;
}

int main(int argc, char *argv[]) {
  while (true) {
    double a;
    scanf("%lf", &a);
    // newton(cos, [](double x){return -sin(x);}, a, 20);
    // newton(log, [](double x){return 1 / x;}, a, 20);
    newton([](double x){ return x*log(x);}, [](double x){ return log(x)+1;}, a, 20);
  }
  return 0;
}
