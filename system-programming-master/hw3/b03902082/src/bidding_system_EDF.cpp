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
  return ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

class Task{
  public:
    double deadline;
    struct timespec rem;
    int type, serial;
    bool started;
    Task(){}
    Task(double deadline, long sec, long nsec, int type, int serial, bool started = false)
      : deadline(deadline)
      , type(type)
      , serial(serial)
      , started(started)
    {
      rem.tv_sec = sec;
      rem.tv_nsec = nsec;
    }

    class Compare{
      public:
        bool operator () (const Task& a, const Task& b){
          return a.deadline >= b.deadline;
        }
    };
};

priority_queue<Task, vector<Task>, Task::Compare> tasks;

int logfd;
int ordinaryCount = 0, memberCount = 0, vipCount = 0;
int cpid;

void safeWrite(int fd, const char * buf, int len){
  sigset_t newset, oldset;
  sigemptyset(&newset);
  sigaddset(&newset, SIGUSR1);
  sigaddset(&newset, SIGUSR2);
  sigaddset(&newset, SIGUSR3);
  sigprocmask(SIG_BLOCK, &newset, &oldset);
  write(fd, buf, len);
  sigprocmask(SIG_SETMASK, &oldset, NULL);
}

int customerCount[3];

void ordinaryHandler(int signo){
  char buffer[100];
  int len, savedErrno = errno;
  ++customerCount[0];
  tasks.emplace(getTime() + 2.0, 0, 500000000, 0, customerCount[0]);
  errno = savedErrno;
}

void patientHandler(int signo){
  char buffer[100];
  int len, savedErrno = errno;
  ++customerCount[1];
  tasks.emplace(getTime() + 3.0, 1, 0, 1, customerCount[1]);
  errno = savedErrno;
}

void impatientHandler(int signo){
  char buffer[100];
  int len, savedErrno = errno;
  ++customerCount[2];
  tasks.emplace(getTime() + 0.3, 0, 200000000, 2, customerCount[2]);
  errno = savedErrno;
}

/*
 * ordinary  0.5 2.0
 * patient   1.0 3.0
 * impatient 0.2 0.3
 *
 */

int main(int argc, char *argv[])
{
  logfd = open("bidding_system_log", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);

  clock_gettime(CLOCK_REALTIME, &timeFrame);
  struct sigaction act;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGUSR1);
  sigaddset(&act.sa_mask, SIGUSR2);
  sigaddset(&act.sa_mask, SIGUSR3);
  act.sa_handler = ordinaryHandler;
  sigaction(SIGUSR1, &act, NULL);
  act.sa_handler = patientHandler;
  sigaction(SIGUSR2, &act, NULL);
  act.sa_handler = impatientHandler;
  sigaction(SIGUSR3, &act, NULL);

  char * testdataPath = argv[1];
  int fd1[2], fd2[2];
  pipe(fd1);
  pipe(fd2);
  if((cpid = fork()) == 0){
    /* Child */
    close(logfd);
    close(0);
    close(1);
    dup2(fd1[1], 1);
    dup2(fd2[0], 0);
    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);
    execl("customer_EDF", "./customer_EDF", testdataPath, NULL);
    perror("customer_EDF");
    exit(1);
  }
  close(fd1[1]);
  close(fd2[0]);

  int customerReadfd = fd1[0], customerWritefd = fd2[1];

  /* End of Prologue*/

  sigset_t mask, oldset;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigaddset(&mask, SIGUSR3);

  char buffer[100];
  int len;
  bool readyToTerm = false;

  struct timeval tv;
  fd_set readset;

  while(true){
    sigprocmask(SIG_BLOCK, &mask, &oldset);
    if(!tasks.empty()){
      Task task = tasks.top(); tasks.pop();
      struct timespec rem = task.rem, req;
      int type = task.type, serial = task.serial;
      if(! task.started){
        len = snprintf(buffer, 100, "receive %d %d\n", type, serial);
        safeWrite(logfd, buffer, len);
      }
      sigprocmask(SIG_SETMASK, &oldset, NULL);
      if(nanosleep(&rem, &req) != 0){
        sigprocmask(SIG_BLOCK, &mask, &oldset);
        tasks.emplace(task.deadline, req.tv_sec, req.tv_nsec, type, serial, true);
        sigprocmask(SIG_SETMASK, &oldset, NULL);
        continue;
      }
      sigprocmask(SIG_BLOCK, &mask, &oldset);
      switch(type){
        case 0:
          kill(cpid, SIGUSR1);
          break;
        case 1:
          kill(cpid, SIGUSR2);
          break;
        case 2:
          kill(cpid, SIGUSR3);
          break;
        default:
          exit(1);
      }
      len = snprintf(buffer, 100, "finish %d %d\n", type, serial);
      safeWrite(logfd, buffer, len);
      sigprocmask(SIG_SETMASK, &oldset, NULL);
      continue;
    }
    sigprocmask(SIG_SETMASK, &oldset, NULL);
    if(readyToTerm){
      break;
    }else{
      tv.tv_sec = 0;
      tv.tv_usec = 1;
      FD_ZERO(&readset);
      FD_SET(customerReadfd, &readset);
      int retval = select(customerReadfd+1, &readset, NULL, NULL, &tv);
      if(retval == -1 || retval == 0){
        // nothing to read from customerRead
        // or interrupted
        continue;
      }else{
        errno = 0;
        if(read(customerReadfd, buffer, 100) == NULL){
          if(errno == EINTR){
            continue;
          }else{
            // EOF
            readyToTerm = true;
          }
        }
        if(strcmp(buffer, "terminate\n") == 0){
          readyToTerm = true;
        }
      }
    }
  }
  len = snprintf(buffer, 100, "terminate\n");
  safeWrite(logfd, buffer, len);
  return 0;
}

