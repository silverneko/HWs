/* b03902082 江懿友 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <queue>

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

class Host{
  public:
    bool idle;
    int readfd;
    int writefd;
    Host(){}
    Host(int fd1, int fd2) : idle(true), readfd(fd1), writefd(fd2) {}
};

Host deployHost(int num);
vector<tuple<int, int, int, int>> combinations(int n);

int main(int argc, char *argv[]){
  if(argc < 3){
    die("[bidding system] Wrong number of arguments");
    exit(1);
  }

  int hostNum = atoi(argv[1]);
  int playerNum = atoi(argv[2]);

  if(hostNum < 1){
    die("[bidding system] Bad host num");
  }
  if(playerNum < 4){
    die("[bidding system] Bad player num");
  }

  Host hosts[MAXHOST];
  for(int i = 1; i <= hostNum; ++i){
    hosts[i] = deployHost(i);
  }

  struct timeval timeout;
  fd_set master_set, working_set;
  FD_ZERO(&master_set);
  int maxfd = getdtablesize();

  int gameResult[MAXPLAYER] = {0};
  auto games = combinations(playerNum);
  auto game = games.begin();
  char buffer[50];
  while(game != games.end()){
    for(int i = 1; game != games.end() && i <= hostNum; ++i){
      if(! hosts[i].idle){
        continue;
      }
      hosts[i].idle = false;
      int a, b, c, d;
      tie(a, b, c, d) = *game++;
      int len = snprintf(buffer, 50, "%d %d %d %d\n", a, b, c, d);
      write(hosts[i].writefd, buffer, len);
      FD_SET(hosts[i].readfd, &master_set);
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    memcpy(&working_set, &master_set, sizeof(master_set));
    select(maxfd, &working_set, NULL, NULL, &timeout);
    for(int i = 1; i <= hostNum; ++i){
      Host host = hosts[i];
      int fd = host.readfd;
      if(! FD_ISSET(fd, &working_set)){
        continue;
      }
      FD_CLR(fd, &master_set);
      hosts[i].idle = true;
      int len = read(fd, buffer, 50);
      buffer[len] = '\0';
      int a, b, c, d;
      int ra, rb, rc, rd;
      sscanf(buffer, "%d %d %d %d %d %d %d %d", &a, &ra, &b, &rb, &c, &rc, &d, &rd);
      gameResult[a] += 4 - ra;
      gameResult[b] += 4 - rb;
      gameResult[c] += 4 - rc;
      gameResult[d] += 4 - rd;
    }
  }
  for(int i = 1; i <= hostNum; ++i){
    Host host = hosts[i];
    int fd = host.readfd;
    if(host.idle){
      continue;
    }
    hosts[i].idle = true;
    int len = read(fd, buffer, 50);
    buffer[len] = '\0';
    int a, b, c, d;
    int ra, rb, rc, rd;
    sscanf(buffer, "%d %d %d %d %d %d %d %d", &a, &ra, &b, &rb, &c, &rc, &d, &rd);
    gameResult[a] += 4 - ra;
    gameResult[b] += 4 - rb;
    gameResult[c] += 4 - rc;
    gameResult[d] += 4 - rd;
  }

  for(int i = 1; i <= hostNum; ++i){
    Host host = hosts[i];
    write(host.writefd, "-1 -1 -1 -1\n", 12);
  }

  int _score[MAXPLAYER];
  int rank[MAXPLAYER];
  for(int i = 0; i < playerNum; ++i){
    _score[i] = gameResult[i+1];
  }
  sort(_score, _score + playerNum);
  reverse(_score, _score + playerNum);
  for(int i = 0; i < playerNum; ++i){
    if(i != 0 && _score[i] == _score[i-1]) continue;
    for(int j = 1; j <= playerNum; ++j){
      if(gameResult[j] == _score[i]){
        rank[j] = i+1;
      }
    }
  }
  for(int i = 1; i <= playerNum; ++i){
    cout << i << " " << rank[i] << endl;
  }
  return 0;
}

Host deployHost(int num){
  /* Open pipes for communications with child */
  int fd[2];
  pipe(fd);
  int readfd = fd[0];
  int parentWritefd = fd[1];
  pipe(fd);
  int parentReadfd = fd[0];
  int writefd = fd[1];

  int pid = fork();
  if(pid == 0){
    // Child Process
    /* Close parent's read and write ends */
    close(readfd);
    close(writefd);
    dup2(parentReadfd, STDIN_FILENO);
    dup2(parentWritefd, STDOUT_FILENO);
    close(parentWritefd);
    close(parentReadfd);
    char hostid[10];
    snprintf(hostid, 10, "%d", num);
    execl("./host", "./host", hostid, NULL);
    die("[bidding system] execl fail");
  }else{
    /* Close child's read and write ends */
    close(parentWritefd);
    close(parentReadfd);
    return Host(readfd, writefd);
  }
}

vector<tuple<int, int, int, int>> combinations(int n){
  vector<tuple<int, int, int, int>> res;
  for(int a = 1; a <= n; ++a){
    for(int b = a+1; b <= n; ++b){
      for(int c = b+1; c <= n; ++c){
        for(int d = c+1; d <= n; ++d){
          res.emplace_back(a, b, c, d);
        }
      }
    }
  }
  return res;
}

