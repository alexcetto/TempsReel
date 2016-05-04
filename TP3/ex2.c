/*******************************************************
 * ex2 : démo de l'utilisation de signaux et de timers
 * (c) F. Touchard 2011
 ******************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#define MYSIGALARM SIGRTMAX
  
struct timespec mywaitime;
struct itimerspec setting;

int handler_execute = 0;
int nb;

void fin(int num, siginfo_t *info, void * foo)
{
  nb = num;
  handler_execute = 1;
}

main (int argc, char **argv)
{
  struct sigaction sa;
  timer_t my_alarm;
  struct sigevent sigspec;
  union sigval value;
  int status;
  sigset_t sigset;


  if (argc < 2) {
    printf("syntax : ex2 measure_time [sec]\n");
    exit(1);
  }

//
  setting.it_value.tv_sec = atoi(argv[1]);
  setting.it_value.tv_nsec = 0;
  setting.it_interval.tv_sec = 0;
  setting.it_interval.tv_nsec = 0;
  
// 
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = fin;

//
  if (sigaction(MYSIGALARM, &sa, NULL) < 0) {
    perror("sigaction MYSIGALARM");
    exit(1);
  }

//
  sigspec.sigev_signo = MYSIGALARM;
  sigspec.sigev_notify = SIGEV_SIGNAL;
  sigspec.sigev_value.sival_int = 1;
  
//
  if (timer_create(CLOCK_REALTIME, &sigspec, &my_alarm) != 0) {
    perror("timer_create");
    exit(1);
  }

//
  if (timer_settime(my_alarm, 0, &setting, NULL) != 0) {
    perror("timer_settime");
    exit(1);
  }

//
  sigemptyset(&sigset);
//
  sigaddset(&sigset, MYSIGALARM);
//
  status = sigwaitinfo(&sigset, NULL);
  printf("recu le signal # %d, handler_execute = %d status = %d\n", nb, handler_execute,status);
  if (status == -1) perror("sigsuspend");
  
}



