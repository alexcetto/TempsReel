#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

pthread_t tid;
pthread_attr_t attr;

void thread_fils(void *foo) {
	printf("--> J'ai pour argument %d \n", *((int *)foo));
	sleep(3);
	pthread_exit((void *)pthread_self());
}

int main() {
	int err;
	void *status;
	unsigned long idt;
	int * arg = malloc(sizeof(*arg));
	*arg = 42;
	
	err = pthread_attr_init(&attr);
	if (err) {
		printf("erreur pthread_attr_init\n");
		exit(EXIT_FAILURE);
	}
	printf("--> Je donne pour argument %d \n", *arg); 
	
	err = pthread_create(&tid, &attr, (void *) thread_fils, arg); // Argument passé dans arg
	if (err) {
		printf("erreur create\n");
		exit (EXIT_FAILURE);
	}
	
	err = pthread_join(tid, &status);
	if (err) {
		printf("erreur join\n");
		exit (EXIT_FAILURE);
	}
	idt = (unsigned long) status;
}
