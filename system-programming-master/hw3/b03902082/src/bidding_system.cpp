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
int ordinaryCount = 0, memberCount = 0, vipCount = 0;
int cpid;

void safeWrite(int fd, const char * buf, int len){
  sigset_t newset, oldset;
  sigemptyset(&newset);
  sigaddset(&newset, SIGUSR1);
  sigaddset(&newset, SIGUSR2);
  sigprocmask(SIG_BLOCK, &newset, &oldset);
  write(fd, buf, len);
  sigprocmask(SIG_SETMASK, &oldset, NULL);
}

void memberHandler(int signo){
  char buffer[100];
  int len, savedErrno = errno;
  ++memberCount;
  len = snprintf(buffer, 100, "receive %d %d\n", 1, memberCount);
  safeWrite(logfd, buffer, len);
  struct timespec req, rem;
  req.tv_sec = 0;
  req.tv_nsec = 500000000;
  while(nanosleep(&req, &rem) != 0){
    req.tv_sec = rem.tv_sec;
    req.tv_nsec = rem.tv_nsec;
  }
  kill(cpid, SIGUSR1);
  len = snprintf(buffer, 100, "finish %d %d\n", 1, memberCount);
  safeWrite(logfd, buffer, len);
  errno = savedErrno;
}

void vipHandler(int signo){
  char buffer[100];
  int len, savedErrno = errno;
  ++vipCount;
  len = snprintf(buffer, 100, "receive %d %d\n", 2, vipCount);
  safeWrite(logfd, buffer, len);
  struct timespec req, rem;
  req.tv_sec = 0;
  req.tv_nsec = 200000000;
  while(nanosleep(&req, &rem) != 0){
    req.tv_sec = rem.tv_sec;
    req.tv_nsec = rem.tv_nsec;
  }
  kill(cpid, SIGUSR2);
  len = snprintf(buffer, 100, "finish %d %d\n", 2, vipCount);
  safeWrite(logfd, buffer, len);
  errno = savedErrno;
}

int main(int argc, char *argv[])
{
  logfd = open("bidding_system_log", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);

  struct sigaction act;
  act.sa_handler = memberHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGUSR1, &act, NULL);
  act.sa_handler = vipHandler;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGUSR1);
  act.sa_flags = 0;
  sigaction(SIGUSR2, &act, NULL);

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
    execl("customer", "./customer", testdataPath, NULL);
    perror("customer");
    exit(1);
  }
  close(fd1[1]);
  close(fd2[0]);

  int customerReadfd = fd1[0], customerWritefd = fd2[1];
  FILE * customerRead = fdopen(customerReadfd, "r");

  /* End of Prologue*/

  clock_gettime(CLOCK_REALTIME, &timeFrame);
  char buffer[100];
  int len;
  while(true){
    errno = 0;
    if(fgets(buffer, 100, customerRead) == NULL){
      if(errno == EINTR){
        continue;
      }
      len = snprintf(buffer, 100, "terminate\n");
      safeWrite(logfd, buffer, len);
      break;
    }
    if(strcmp(buffer, "ordinary\n") == 0){
      /* received ordinary */
      ordinaryCount++;
      len = snprintf(buffer, 100, "receive %d %d\n", 0, ordinaryCount);
      safeWrite(logfd, buffer, len);
      struct timespec req, rem;
      req.tv_sec = 1;
      req.tv_nsec = 0;
      while(nanosleep(&req, &rem) != 0){
        req.tv_sec = rem.tv_sec;
        req.tv_nsec = rem.tv_nsec;
      }
      kill(cpid, SIGINT);
      len = snprintf(buffer, 100, "finish %d %d\n", 0, ordinaryCount);
      safeWrite(logfd, buffer, len);
    }
  }
  return 0;
}

