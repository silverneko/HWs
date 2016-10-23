#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <queue>

#define SIGUSR3 SIGWINCH

using namespace std;

struct timespec timeFrame;

double getTime(){
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec -= timeFrame.tv_sec;
  ts.tv_nsec -= timeFrame.tv_nsec;
  while(ts.tv_nsec < 0){
    ts.tv_sec -= 1;
    ts.tv_nsec += 1000000000;
  }
  double res = ts.tv_sec;
  res += (double)ts.tv_nsec / 1e9;
  return res;
}

int logfd;

void safeWrite(int fd, const char * buf, int len){
  sigset_t newset, oldset;
  sigemptyset(&newset);
  sigaddset(&newset, SIGINT);
  sigaddset(&newset, SIGUSR1);
  sigaddset(&newset, SIGUSR2);
  sigprocmask(SIG_BLOCK, &newset, &oldset);
  write(fd, buf, len);
  sigprocmask(SIG_SETMASK, &oldset, NULL);
}

bool done[3][10000000];
int num[3];

queue<int> numq;

void signalHandler(int signo){
  char buffer[100];
  int len, nu;
  switch(signo){
    case SIGINT:
      nu = numq.front();
      numq.pop();
      done[0][nu] = true;
      len = snprintf(buffer, 100, "finish %d %d\n", 0, nu);
      safeWrite(logfd, buffer, len);
      break;
    case SIGUSR1:
      done[1][num[1]] = true;
      len = snprintf(buffer, 100, "finish %d %d\n", 1, num[1]);
      safeWrite(logfd, buffer, len);
      break;
    case SIGUSR2:
      done[2][num[2]] = true;
      len = snprintf(buffer, 100, "finish %d %d\n", 2, num[2]);
      safeWrite(logfd, buffer, len);
      break;
    default:
      break;
  }
}

class Task{
  public:
    int type, serial;
    double stime;
    bool timeout;
    Task(){}
    Task(int type, int serial, double t, bool f = false) : type(type), serial(serial), stime(t), timeout(f) {}
};

int main(int argc, char *argv[])
{
  logfd = open("customer_log", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);

  struct sigaction act;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGINT);
  sigaddset(&act.sa_mask, SIGUSR1);
  sigaddset(&act.sa_mask, SIGUSR2);
  act.sa_flags = 0;
  act.sa_handler = signalHandler;
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);

  char * testdataPath = argv[1];
  /* End of Prologue*/

  FILE * td = fopen(testdataPath, "r");
  int type;
  double stime;
  char buffer[100];
  int len;
  int count[3] = {0};
  int ppid = getppid();

  vector<Task> tasks;
  while(fscanf(td, "%d %lf", &type, &stime) != EOF){
    ++count[type];
    tasks.emplace_back(type, count[type], stime);
    switch(type){
      case 0:
        break;
      case 1:
        tasks.emplace_back(type, count[type], stime + 1.0, true);
        break;
      case 2:
        tasks.emplace_back(type, count[type], stime + 0.3, true);
        break;
      default:
        exit(1);
    }
  }
  sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b){
        return a.stime < b.stime;
      });

  sigset_t mask, oldmask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);

  clock_gettime(CLOCK_REALTIME, &timeFrame);
  for(int i = 0; i < tasks.size(); ++i){
    int type = tasks[i].type;
    int serial = tasks[i].serial;
    double stime = tasks[i].stime;
    bool timeout = tasks[i].timeout;
    while(getTime() < stime){
      double timeWindow = stime - getTime();
      struct timespec rem, req;
      rem.tv_sec = floor(timeWindow);
      rem.tv_nsec = (timeWindow - floor(timeWindow)) * 1e9;
      while(nanosleep(&rem, &req) != 0){
        if(getTime() >= stime){
          break;
        }
        timeWindow = stime - getTime();
        rem.tv_sec = floor(timeWindow);
        rem.tv_nsec = (timeWindow - floor(timeWindow)) * 1e9;
      }
    }
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    if(timeout){
      if(!done[type][serial]){
        len = snprintf(buffer, 100, "timeout %d %d\n", type, serial);
        safeWrite(logfd, buffer, len);
        exit(0);
      }
    }else{
      switch(type){
        case 0:
          len = snprintf(buffer, 100, "ordinary\n");
          safeWrite(1, buffer, len);
          break;
        case 1:
          kill(ppid, SIGUSR1);
          break;
        case 2:
          kill(ppid, SIGUSR2);
          break;
        default:
          exit(1);
      }
      if(type == 0){
        numq.push(serial);
      }else{
        num[type] = serial;
      }
      done[type][serial] = false;
      len = snprintf(buffer, 100, "send %d %d\n", type, serial);
      safeWrite(logfd, buffer, len);
    }
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
  }
  while(!numq.empty());

  return 0;
}

