/*******************************************************
 * ex1 : démo de l'utilisation de signaux et de timers
 * (c) F. Touchard 2011

 Commenté par :
 Alexandre Lagarange-Cetto
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

/*
  sigemptyset initialise le masque de sigaction à vide, tout signal exclu
  Il est bon de noté que sigemptyset est MT-Safe | AS-Safe | AC-Safe
  les flags sont aussi définis à zéro
  Le handler est défini avec la fonction fin_mesure
*/
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = fin_mesure;            

/*
  Sigaction change sa pour le signal de timer
*/
  if (sigaction(MYSIGALARM, &sa, NULL) < 0) {
    perror("sigaction MYSIGALARM");
    exit(1);
  }

// Récupère l'argument de main et lance alarm(argv[1]), qui envoie SIGALRM au bout de argv[1] s.
  alarm(atoi(argv[1]));

/*
  Vide sigset 
  Et le redéfini avec -1 de sigsuspend. 
  Sigsuspend mets le process en hibernation tant qu'il ne reçoit pas de signal non bloqué
*/
  sigemptyset(&sigset);
  status = sigsuspend(&sigset);
  printf("recu le signal # %d, handler_execute = %d status = %d\n", signum, handler_execute,status);

}


void fin_mesure(int num)
{
  signum = num;
  handler_execute = 1;
}

