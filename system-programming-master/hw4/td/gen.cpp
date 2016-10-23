#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>

using namespace std;

int main(int argc, char *argv[])
{
  int n;
  cin >> n;
  cout << n << endl;
  for(int i = 0; i < n; ++i){
    unsigned hi = rand() << 15, lo = rand();
    unsigned a = (hi + lo) % (5 * 10000000);
    cout << a << ' ';
  }
  cout << endl;
  return 0;
}

