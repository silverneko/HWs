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
#include <sys/wait.h>
#include <fcntl.h>
#include <ctime>

#define MAXHOST 15
#define MAXPLAYER 25

using namespace std;

void die(const char * msg){
  cerr << msg << endl;
  exit(1);
}

#define BUFSIZE 50

char buffer[BUFSIZE];

class Player{
  public:
    int fd;
    int money;
    int rkey;
    Player(){}
    Player(int fd, int money, int rkey) : fd(fd), money(money), rkey(rkey) {}
};

Player deployPlayer(int hostId, int id, int rkey);
bool isunique(int x, int * _begin, int * _end);

int main(int argc, char *argv[]){
  if(argc < 2){
    die("[host] Wrong number of arguments");
  }

  int hostId = atoi(argv[1]);

  char pipeNames[5][50] = {0};
  const char * _pipeNames[5] = {"_A", "_B", "_C", "_D", ""};
  for(int i = 0; i <= 4; ++i){
    snprintf(pipeNames[i], 50, "host%d%s.FIFO", hostId, _pipeNames[i]);
  }
  srand(time(NULL));
  int rkeys[65536];
  for(int i = 0; i < 65536; ++i){
    rkeys[i] = i;
  }

  while(true){
    int id[4];
    for(int i = 0; i < 4; ++i){
      cin >> id[i];
    }
    if(id[0] == -1 && id[1] == -1 && id[2] == -1 && id[3] == -1){
      break;
    }

    for(int i = 0; i <= 4; ++i){
      mkfifo(pipeNames[i], S_IRWXU);
    }

    Player players[4];
    random_shuffle(rkeys, rkeys + 65536);
    for(int i = 0; i < 4; ++i){
      players[i] = deployPlayer(hostId, i, rkeys[i]);
      players[i].fd = open(pipeNames[i], O_WRONLY);
    }
    int readfd = open(pipeNames[4], O_RDONLY);

    int score[4] = {0};
    for(int i = 0; i < 10; ++i){
      for(int j = 0; j < 4; ++j){
        players[j].money += 1000;
      }
      int bid[4];
      char _buffer[200];
      int len = snprintf(buffer, BUFSIZE, "%d %d %d %d\n", players[0].money, players[1].money, players[2].money, players[3].money);
      for(int j = 0; j < 4; ++j){
        write(players[j].fd, buffer, len);
        int len = read(readfd, _buffer, 200);
        _buffer[len] = '\0';
        char playerIndex[5];
        int randomKey, money;
        sscanf(_buffer, "%s %d %d", playerIndex, &randomKey, &money);
        bid[playerIndex[0] - 'A'] = money;
      }
      int _bid[4];
      for(int j = 0; j < 4; ++j){
        _bid[j] = bid[j];
      }
      sort(_bid, _bid + 4);
      reverse(_bid, _bid + 4);
      int winningBid = -1;
      for(int j = 0; j < 4; ++j){
        if(isunique(_bid[j], bid, bid+4)){
          winningBid = _bid[j];
          break;
        }
      }
      if(winningBid != -1){
        /* Well well well. We have a winner over here. */
        for(int j = 0; j < 4; ++j){
          if(bid[j] == winningBid){
            players[j].money -= bid[j];
            ++score[j];
            break;
          }
        }
      }
    }

    int _score[4];
    int rank[4];
    for(int i = 0; i < 4; ++i){
      _score[i] = score[i];
    }
    sort(_score, _score + 4);
    reverse(_score, _score + 4);
    for(int i = 0; i < 4; ++i){
      if(i != 0 && _score[i] == _score[i-1]) continue;
      for(int j = 0; j < 4; ++j){
        if(score[j] == _score[i]){
          rank[j] = i+1;
        }
      }
    }

    int len = snprintf(buffer, BUFSIZE, "%d %d\n%d %d\n%d %d\n%d %d\n", id[0], rank[0], id[1], rank[1], id[2], rank[2], id[3], rank[3]);
    write(1, buffer, len);

    close(readfd);
    for(int i = 0; i < 4; ++i){
      close(players[i].fd);
    }
    for(int i = 0; i < 4; ++i){
      wait(NULL);
    }
    for(int i = 0; i <= 4; ++i){
      remove(pipeNames[i]);
    }
  }

  return 0;
}

bool isunique(int x, int * _begin, int * _end){
  int count = 0;
  while(_begin != _end){
    if(x == *_begin){
      ++count;
    }
    ++_begin;
  }
  if(count == 1) return true;
  else return false;
}

Player deployPlayer(int hostId, int id, int rkey){
  int pid = fork();
  if(pid == 0){
    /* Child */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    char host_id[20], name[20], random_key[20];
    snprintf(host_id, 20, "%d", hostId);
    snprintf(name, 20, "%c", 'A' + id);
    snprintf(random_key, 20, "%d", rkey);
    execl("./player", "./player", host_id, name, random_key, NULL);
    die("[host] execl error");
  }else{
    return Player(0, 0, rkey);
  }
}

