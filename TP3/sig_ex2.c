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
#define MYSIG2D 		SIGRTMAX
#define MYSIGSTOP 		SIGRTMAX-1
#define MYSIGALARM	 	SIGRTMAX-2
#define COMPTE_SIGNAUX 	1
#define TERMINE_FILS 	2
#define FIN_MESURE 		3

int nsigs = 0;
pid_t chpid;
int mesure=1;
struct itimerspec setting;

main (int argc, char **argv)
{
	struct sigaction sa;
	//extern void compte_signaux(), fin_mesure(), termine_fils();
	extern void newHandler(int action);
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
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = newHandler;            /* Fin de la mesure */


	if (sigaction(MYSIGALARM, &sa, NULL) < 0) {
		perror("sigaction MYSIGALARM");
		exit(1);
	}

// PLUS BESOIN DE ÇA
	/**
	sa.sa_sigaction = compte_signaux;           
	if (sigaction(MYSIG2D, &sa, NULL) < 0) {
		perror("sigaction MYSIG2D");
	}

	sa.sa_sigaction = termine_fils;           
	sigfillset(&sa.sa_mask);                
	if (sigaction(MYSIGSTOP, &sa, NULL) < 0) {
		perror("sigaction MYSIGSTOP");
	}

	*/

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
	union sigval value;

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
		//if (kill(chpid, MYSIG2D) < 0) {
		if (sigqueue(chpid, MYSIG2D, value))
		{
			perror("kill");
			return;
		}
		nsigs++;
	}
	printf("%d signaux envoyes par le pere\n",nsigs);
	fflush(stdout);
	// kill(chpid, MYSIGSTOP);

	sigqueue(chpid, MYSIGSTOP, value);

	wait(&status);
	exit(0);

}

void newHandler(int action){
	switch(action){
		case COMPTE_SIGNAUX:
			nsigs++;
			break;
		
		case TERMINE_FILS:
			mesure = 0;
			break;

		case FIN_MESURE:
			mesure = 0;
			break;

		default:
			exit(0);
			break;
	}
}


/** PLUS UTILE
void compte_signaux()
{
	nsigs++;
}

void termine_fils() 
{
	mesure = 0;
}

void fin_mesure()
{
	mesure = 0;
}

*/