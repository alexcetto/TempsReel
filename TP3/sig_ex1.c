/*******************************************************
* sig_ex : envoi d'un signal d'un process vers un autre
* (c) F. Touchard 2010
******************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#define MYSIG2D SIGRTMAX
#define MYSIGSTOP SIGRTMAX-1
#define MYSIGALARM SIGRTMAX-2

int nsigs = 0;
pid_t chpid;
int mesure=1;
struct itimerspec setting;

main (int argc, char **argv)
{
	struct sigaction sa;
	extern void compte_signaux(), fin_mesure(), termine_fils();
	sigset_t blockem;

	if (argc < 2) {
		printf("syntax : sig_ex1 measure_time [sec]\n");
		exit(1);
	}

	setting.it_value.tv_sec = atoi(argv[1]);
	setting.it_value.tv_nsec = 0;
	setting.it_interval.tv_sec = 0;
	setting.it_interval.tv_nsec = 0;

	sigemptyset(&blockem);
	sigaddset(&blockem, MYSIG2D);
	sigprocmask(SIG_BLOCK, &blockem, NULL);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = fin_mesure;            /* Fin de la mesure */

	if (sigaction(MYSIGALARM, &sa, NULL) < 0) {
		perror("sigaction MYSIGALARM");
		exit(1);
	}

	sa.sa_handler = compte_signaux;           /* Comptage du nombre de signaux recus */
	if (sigaction(MYSIG2D, &sa, NULL) < 0) {
		perror("sigaction MYSIG2D");
	}

	sa.sa_handler = termine_fils;           /* Fin du processus fils */
	sigfillset(&sa.sa_mask);                /* Tous les signaux bloques apres la fin de la mesure */
	if (sigaction(MYSIGSTOP, &sa, NULL) < 0) {
		perror("sigaction MYSIGSTOP");
	}

	switch (chpid = fork()) {
		case -1: /* erreur */
			perror("fork");
			exit(2);
			break;
		case 0:  /* child */
			proc_fils();
			exit(0);
			break;
		default:   /* parent */
			proc_pere();
			exit(0);
			break;
	}
	fprintf(stderr, "Sortie anormale du programme\n");
	exit(3);
}

proc_fils()
{
	sigset_t sigset;
	sigemptyset(&sigset);
	while (mesure) {
		sigsuspend(&sigset);
	}

	printf("%d signaux recus par le fils\n",nsigs);
	fflush(stdout);
	exit(0);
}

proc_pere()
{
	timer_t my_alarm;
	struct sigevent sigspec;
	int status;

	sigspec.sigev_signo = MYSIGALARM;
	sigspec.sigev_notify = SIGEV_SIGNAL;
	sigspec.sigev_value.sival_int = 1;

	if (timer_create(CLOCK_REALTIME, &sigspec, &my_alarm) != 0) {
		perror("timer_create");
		exit(1);
	}
	#ifdef DEBUG
		printf("meas time %d\n", setting.it_value.tv_sec);
	#endif
	if (timer_settime(my_alarm, 0, &setting, NULL) != 0) {
		perror("timer_settime");
		exit(1);
	}

	while(mesure) {
		if (kill(chpid, MYSIG2D) < 0) {
			perror("kill");
			return;
		}
		nsigs++;
	}
	printf("%d signaux envoyes par le pere\n",nsigs);
	fflush(stdout);
	kill(chpid, MYSIGSTOP);
	wait(&status);
	exit(0);

}

void compte_signaux()
{
	tv_nsecigs++;
}

void termine_fils() 
{
	mesure = 0;
}

void fin_mesure()
{
	mesure = 0;
}

