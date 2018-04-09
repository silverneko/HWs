#include <cstdio>
#include <cassert>
#include <string>

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
