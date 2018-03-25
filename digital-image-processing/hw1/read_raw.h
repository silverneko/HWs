#include <cstdio>
#include <cassert>

void read_raw(const char *pathname, unsigned int size, void *buf) {
  FILE *infile;
  infile = fopen(pathname, "rb");
  assert(infile != NULL);
  fread(buf, sizeof(unsigned char), size, infile);
  fclose(infile);
  return;
}

void save_raw(const char *pathname, unsigned int size, void *buf) {
  FILE *file;
  file = fopen(pathname, "wb");
  assert(file != NULL);
  fwrite(buf, sizeof(unsigned char), size, file);
  fclose(file);
  return;
}
