#include <cstdio>
#include <cmath>
#include <functional>

template<typename F>
double bisec(F f, double a0, double b0, unsigned maxIter = 10) {
  if (f(a0) * f(b0) > 0) {
    printf("f(%.10e) and f(%.10e) have same sign.\n", a0, b0);
    return a0;
  }
  double m;
  for (int i = 0; i < maxIter; ++i) {
    m = (a0 + b0) / 2;
    if (f(m) * f(a0) < 0) {
      b0 = m;
    } else {
      a0 = m;
    }
    printf("#%d\t| m = %.10e | f(m) = %.10e\n", i+1, m, f(m));
  }
  return m;
}

int main(int argc, char *argv[]) {
  while (true) {
    double a, b;
    scanf("%lf %lf", &a, &b);
    // bisec([](double x) { return cos(x);}, a, b, 20);
    // bisec([](double x) { return log(x);}, a, b, 20);
    bisec([](double x) { return x*log(x);}, a, b, 20);
  }
  return 0;
}
