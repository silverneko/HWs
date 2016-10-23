/* b03902082 江懿友 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <queue>
#include <sstream>

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>

#define MAXHOST 15
#define MAXPLAYER 25

using namespace std;

void die(const char * msg){
  cerr << msg << endl;
  exit(1);
}

#define BUFSIZE 50

char buffer[BUFSIZE];

int main(int argc, char *argv[]){
  if(argc < 4){
    die("[player] Wrong number of arguments");
  }

  int hostId = atoi(argv[1]);
  char * playerIndex = argv[2];
  char * randomKey = argv[3];

  int playeri = playerIndex[0] - 'A';
  int readfd, writefd;
  snprintf(buffer, BUFSIZE, "host%d_%s.FIFO", hostId, playerIndex);
  readfd = open(buffer, O_RDONLY);
  snprintf(buffer, BUFSIZE, "host%d.FIFO", hostId);
  writefd = open(buffer, O_WRONLY);

  for(int i = 0; i < 10; ++i){
    int len = read(readfd, buffer, BUFSIZE);
    buffer[len] = '\0';
    int money[4];
    sscanf(buffer, "%d %d %d %d", &money[0], &money[1], &money[2], &money[3]);
    if((i % 4) == playeri){
      len = snprintf(buffer, BUFSIZE, "%s %s %d\n", playerIndex, randomKey, money[playeri]);
    }else{
      len = snprintf(buffer, BUFSIZE, "%s %s %d\n", playerIndex, randomKey, 0);
    }
    write(writefd, buffer, len);
  }

  close(writefd);
  close(readfd);
  return 0;
}

