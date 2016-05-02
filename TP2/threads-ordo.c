#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>

#define NB_THREADS 5

pthread_t tid[NB_THREADS];
pthread_attr_t attr[NB_THREADS];

void thread_fils(int foo) {
	sleep(3);
	printf("--> J'ai pour argument %d \n", foo);
	pthread_exit((void *)pthread_self());
}

int main() {
	int err, i;
	void *status;

	for(i = 0; i<NB_THREADS; i++){
		struct sched_param param; // Définie dans sched.h
		
		err = pthread_attr_init(&attr[i]);
		if (err) {
			printf("erreur pthread_attr_init for thread %d.\n", i);
			exit(EXIT_FAILURE);
		}

		err = pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
		if (err){
			printf("%s\n", "Error: Set scheduling policy for thread %d.\n", i);
			exit(EXIT_FAILURE);
		}
		// Normalement: [sched_get_priority_min() < sched_priority < sched_get_priority_max()]
		param.sched_priority++; // défini la priorité du thread

		printf("--> Je donne pour argument %d \n", i);
		err = pthread_create(&tid[i], &attr[i], (void*) thread_fils, i);
		if (err) {
			printf("erreur create\n");
			exit (EXIT_FAILURE);
		}
	}

	for (i = 0; i < NB_THREADS; i++){
		err = pthread_join(tid[i], &status);
		if (err) {
			printf("Error: Cannont join thread.\n");
			exit (EXIT_FAILURE);
		}
	}
}
