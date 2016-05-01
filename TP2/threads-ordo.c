#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

pthread_t tid;
pthread_attr_t attr;

void thread_fils(int foo) {
	printf("--> J'ai pour argument %d \n", foo);
	sleep(3);
	pthread_exit((void *)pthread_self());
}

int main() {
	int err;
	void *status;
	unsigned long idt;
	int i;
	int policy = SCHED_FIFO;

	err = pthread_attr_init(&attr);
	if (err) {
		printf("erreur pthread_attr_init\n");
		exit(EXIT_FAILURE);
	}


	for(i = 0; i<5; i++){
		printf("--> Je donne pour argument %d \n", i);
		err = pthread_create(&tid, &attr, (void *) thread_fils, i);
		if (err) {
			printf("erreur create\n");
			exit (EXIT_FAILURE);
		}
	}


	err = pthread_join(tid, &status);
	if (err) {
		printf("erreur join\n");
		exit (EXIT_FAILURE);
	}
	idt = (unsigned long) status;
	//printf("Le pere a synchronise sur la fin du fils (tid %u)  \n\tstatus recu : %u\n",tid, idt);

}
