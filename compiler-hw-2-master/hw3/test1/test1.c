__attribute__((noinline)) int C(int x, int y, int z) {
  return z;
}

__attribute__((noinline)) int B(int x, int y, int z) {
  return C(x, y, z)+x+y+z;
}

__attribute__((noinline)) int A(int x, int y, int z){
  return B(z, y, x)+x*y;
}

__attribute__((noinline)) int run() {
  int s = 1;
  for (int i=0; i<100000; ++i) {
    s += A(i, i+1, i+2);
  }
  return s;
}
