/*******************************************************
 * ex1 : démo de l'utilisation de signaux et de timers
 * (c) F. Touchard 2011
 ******************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#define MYSIGALARM SIGALRM
 
int signum = -1;
int handler_execute=0;

main (int argc, char **argv)
{
  struct sigaction sa;
  extern void fin_mesure();
  sigset_t sigset;
  int status;


  if (argc < 2) {
    printf("syntax : ex1 temps d'attente [sec]\n");
    exit(1);
  }

//  
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = fin_mesure;            

//
  if (sigaction(MYSIGALARM, &sa, NULL) < 0) {
    perror("sigaction MYSIGALARM");
    exit(1);
  }

//
  alarm(atoi(argv[1]));

//  
  sigemptyset(&sigset);
  status = sigsuspend(&sigset);
  printf("recu le signal # %d, handler_execute = %d status = %d\n", signum, handler_execute,status);

}


void fin_mesure(int num)
{
  signum = num;
  handler_execute = 1;
}

