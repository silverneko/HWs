#ifndef DIP_READ_RAW
#define DIP_READ_RAW

#include <cstdio>
#include <cassert>
#include <string>
#include <memory>

namespace {

using std::string;

void read_raw(const string &pathname, unsigned int size, void *buf) {
  FILE *infile;
  infile = fopen(pathname.c_str(), "rb");
  assert(infile != NULL);
  fread(buf, sizeof(unsigned char), size, infile);
  fclose(infile);
  return;
}

void save_raw(const string &pathname, unsigned int size, void *buf) {
  FILE *file;
  file = fopen(pathname.c_str(), "wb");
  assert(file != NULL);
  fwrite(buf, sizeof(unsigned char), size, file);
  fclose(file);
  return;
}

template<typename T>
T clip(T v, T l, T r) {
  return std::min(std::max(l, v), r);
}

template<typename T>
class Array2d {
public:
  int dim[2];
  std::shared_ptr<T> data;

  Array2d() {};
  Array2d(int dim0, int dim1) {
    dim[0] = dim0;
    dim[1] = dim1;
    data = std::shared_ptr<T>(new T[dim0 * dim1], [](T *p){ delete[] p;});
  }

  T& operator()(int x, int y) {
    return data.get()[x * dim[1] + y];
  }

  T* operator[](int x) {
    return data.get() + x * dim[1];
  }
};

template<typename T>
Array2d<T> pad(Array2d<T> I, int nw) {
  const int ni = I.dim[0], nj = I.dim[1];
  Array2d<T> O(ni + nw*2, nj + nw*2);
  for (int i = 0; i < ni; ++i) {
    for (int j = 0; j < nj; ++j) {
      O(i+nw, j+nw) = I(i, j);
    }
  }
  for (int k = 0; k < nw; ++k) {
    for (int i = 0; i < ni; ++i) {
      O(i+nw, k) = O(i+nw, nw);
      O(i+nw, k+nw+nj) = O(i+nw, nw+nj-1);
    }
    for (int j = 0; j < nj; ++j) {
      O(k, j+nw) = O(nw, j+nw);
      O(k+nw+ni, j+nw) = O(nw+ni-1, j+nw);
    }
  }
  for (int i = 0; i < nw; ++i) {
    for (int j = 0; j < nw; ++j) {
      O(i, j) = O(nw, nw);
      O(i, nw+nj+j) = O(nw, nw+nj-1);
      O(nw+ni+i, j) = O(nw+ni-1, nw);
      O(nw+ni+i, nw+nj+j) = O(nw+ni-1, nw+nj-1);
    }
  }
  return O;
}

template<typename T>
Array2d<T> padwith(Array2d<T> I, int nw, const T& pad_value) {
  const int ni = I.dim[0], nj = I.dim[1];
  Array2d<T> O(ni + nw*2, nj + nw*2);
  for (int i = 0; i < ni; ++i) {
    for (int j = 0; j < nj; ++j) {
      O(i+nw, j+nw) = I(i, j);
    }
  }
  for (int k = 0; k < nw; ++k) {
    for (int i = 0; i < ni; ++i) {
      O(i+nw, k) = pad_value;
      O(i+nw, k+nw+nj) = pad_value;
    }
    for (int j = 0; j < nj; ++j) {
      O(k, j+nw) = pad_value;
      O(k+nw+ni, j+nw) = pad_value;
    }
  }
  for (int i = 0; i < nw; ++i) {
    for (int j = 0; j < nw; ++j) {
      O(i, j) = pad_value;
      O(i, nw+nj+j) = pad_value;
      O(nw+ni+i, j) = pad_value;
      O(nw+ni+i, nw+nj+j) = pad_value;
    }
  }
  return O;
}

template<typename T>
Array2d<T> filter(Array2d<T> I, Array2d<T> K) {
  const int ni = I.dim[0], nj = I.dim[1];
  const int nx = ni - (K.dim[0]-1), ny = nj - (K.dim[1]-1);
  Array2d<T> O(nx, ny);
  for (int i = 0; i < nx; ++i) {
    for (int j = 0; j < ny; ++j) {
      T accu = 0;
      for (int ik = 0; ik < K.dim[0]; ++ik) {
        for (int jk = 0; jk < K.dim[1]; ++jk) {
          accu += K(ik, jk) * I(i+ik, j+jk);
        }
      }
      O(i, j) = accu;
    }
  }
  return O;
}

}
#endif
