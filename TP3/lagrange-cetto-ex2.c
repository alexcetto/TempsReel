/*******************************************************
 * ex2 : démo de l'utilisation de signaux et de timers
 * (c) F. Touchard 2011

 Commenté par :
 Alexandre Lagrange-Cetto
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

// Défini le moment en seconde où le timer terminera. 
// Pas d'intervalle définit donc il déclenche une seule fois.
  setting.it_value.tv_sec = atoi(argv[1]);
  setting.it_value.tv_nsec = 0;
  setting.it_interval.tv_sec = 0;
  setting.it_interval.tv_nsec = 0;
  
// action signal info et définition de la fonction fin comme action
// SIGINFO signifie que des signaux POSIX4 ŝont attendus
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = fin;

// Sigaction change sa pour le signal de timer
  if (sigaction(MYSIGALARM, &sa, NULL) < 0) {
    perror("sigaction MYSIGALARM");
    exit(1);
  }

// On définit ici quel type de signal est attendu
  sigspec.sigev_signo = MYSIGALARM;
  sigspec.sigev_notify = SIGEV_SIGNAL;
  sigspec.sigev_value.sival_int = 1;
  
// on crée notre timer, qui n'est pas encore déclenché ici. 
// Son pointeur est enregistré dans my_alarm.
// CLOCK_REALTIME est l'horloge système.
// sigspec est ici l'événement à envoyer quand le timer termine le décompte.
  if (timer_create(CLOCK_REALTIME, &sigspec, &my_alarm) != 0) {
    perror("timer_create");
    exit(1);
  }

// On lance le timer avec les paramètres définis précédemment
  if (timer_settime(my_alarm, 0, &setting, NULL) != 0) {
    perror("timer_settime");
    exit(1);
  }

// On vide sigset pour le préparer à recevoir sa nouvelle valeur
  sigemptyset(&sigset);
// On définit sigset avec la valeur SIGRTMAX, la borne max (donc de basse priorité!)
  sigaddset(&sigset, MYSIGALARM);
// On attend que le signal soit déclenché par le timer. Dès qu'il est intercepté,
// la fonction se termine
  status = sigwaitinfo(&sigset, NULL);
  printf("recu le signal # %d, handler_execute = %d status = %d\n", nb, handler_execute,status);
  if (status == -1) perror("sigsuspend");
  
}