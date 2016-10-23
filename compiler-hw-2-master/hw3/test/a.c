#include <stdio.h>

long f(long);
long g(long, long);

int main(int argc, char * argv[]) {
  volatile long a, b;
  a = 1;
  b = 2;
  a = f(a);
  b = f(b);
  return 0;
}

long f(long a) {
  long b = a * 10007;
  long c;
  g(a, b+1);
  c = 0;
  for (long i = 0; i < b; ++i) {
    c += (b * i) / 3;
  }
  return c;
}

long g(long a, long b) {
  volatile long c;
  c = a + b;
  return c;
}
