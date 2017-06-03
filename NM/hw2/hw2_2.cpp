#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

int main() {
  cout << hex << showbase;
  double zero = 0.0;
  double nzero = -0.0;
  cout << *(long*)&zero << endl;
  cout << *(long*)&nzero << endl;
  cout << sqrt(0.0) << endl;
  cout << zero / nzero << endl;
  return 0;
}
