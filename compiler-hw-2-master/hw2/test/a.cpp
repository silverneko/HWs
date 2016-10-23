#include <cassert>
#include <cstdio>

int main() {
  int a, b, c, d, t1, t2, t3, t4;
  a = 1;
  b = 2;
  c = 3;
  d = 4;

  // here is a CS "a+b"
  t1 = (a+b) * (a+b);
  // CS "t"
  t2 = (a+b) * (a+b);
  b = c + d;
  // another CS "c+d"
  t3 = (a+b) * (c+d);
  // here is a CS "a+b", "t" is not a CS
  t4 = t3;
  t4 = (a+b) * (a+b);
  t3 = t4;
  t4 = t3;

  int * ptr = &a;
  *ptr = 12;
  b = a;

  assert(t1 == 9 && t2 == 9 && t3 == 64 && t4 == 64);

  int arr[100][100] = {0};
  int accu;
  for (int i = 0; i < 100; ++i) {
    for (int j = 0; j < 50; ++j) {
      // loop unrolling
      arr[i][j] += i*i + j + arr[i][j] + arr[i][j];
      ++j;
      arr[i][j] += i*i + j + arr[i][j] + arr[i][j];
    }
  }

  assert(arr[20][30] == 430);
  puts("ok");
  return 0;
}
