#define _GNU_SOURCE

#include <stdio.h>
#include <float.h>
#include <signal.h>
#include <setjmp.h>
#include <fenv.h>

#pragma STDC FENV_ACCESS ON

void show_fe_exceptions(FILE *file) {
  fprintf(file, "IEEE floating-point exception flags raised:");
  if (fetestexcept(FE_INEXACT))
    fprintf(file, " inexact;");
  if (fetestexcept(FE_DIVBYZERO))
    fprintf(file, " divide by zero;");
  if (fetestexcept(FE_UNDERFLOW))
    fprintf(file, " underflow;");
  if (fetestexcept(FE_OVERFLOW))
    fprintf(file, " overflow;");
  if (fetestexcept(FE_INVALID))
    fprintf(file, " invalid;");
  fprintf(file, "\n");
  fprintf(file, "IEEE floating-point exception traps enabled:");
  int excepts = fegetexcept();
  if (excepts & FE_INEXACT)
    fprintf(file, " inexact;");
  if (excepts & FE_DIVBYZERO)
    fprintf(file, " divide by zero;");
  if (excepts & FE_UNDERFLOW)
    fprintf(file, " underflow;");
  if (excepts & FE_OVERFLOW)
    fprintf(file, " overflow;");
  if (excepts & FE_INVALID)
    fprintf(file, " invalid;");
  fprintf(file, "\n");
}

jmp_buf restore_point;

void handler(int sig, siginfo_t *sip, void *uap) {
  unsigned code, addr;
  code = sip->si_code;
  addr = (unsigned)sip->si_addr;
  fprintf(stderr, "fp exception %x at address %x\n", code, addr);
  longjmp(restore_point, 1);
}

int main() {
  double x;

  struct sigaction siga = {
    .sa_sigaction = handler,
    .sa_flags = SA_SIGINFO
  };
  if (sigaction(SIGFPE, &siga, NULL))
    fprintf(stderr, "Failed to set exception handler\n");
  feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);

  x = DBL_MIN;
  printf("min_normal = %g\n", x);
  x = x / 13.0;
  printf("min_normal / 13.0 = %g\n", x);

  x = DBL_MAX;
  printf("max_normal = %g\n", x);
  fexcept_t except_flag;
  fegetexceptflag(&except_flag, FE_ALL_EXCEPT);
  if (!setjmp(restore_point)) {
    x = x * x;
  } else {
    // Recovered from signal handler
    fesetexceptflag(&except_flag, FE_ALL_EXCEPT);
    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
  }
  printf("max_normal * max_normal = %g\n", x);

  show_fe_exceptions(stderr);
  return 0;
}
