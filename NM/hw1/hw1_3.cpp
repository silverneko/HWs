#include <cstdio>
#include <cmath>

int main(int agrc, char *argv[]) {
  double e = exp(-64.0 * 1.0 + 0.0);
  printf("%1.16e\n", 1.0 - 1.0 / (1.0 + e));
  printf("%1.16e\n", e / (1.0 + e));
  printf("%1.16e\n", 1.0 / (1.0 + e));
  return 0;
}
