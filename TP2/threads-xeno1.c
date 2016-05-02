#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

#define NAMELEN 20

pthread_t tid;
pthread_attr_t attr;

void thread_fils(void *foo) {
	int  err, i;
	char thread_name[] = "thread_fils";

	pthread_set_name_np(pthread_self(), thread_name);
	if (err) {
		printf("Error: fils set_name_np\n");
		exit(EXIT_FAILURE);
	}

	printf("Interruption, entrer un nombre :\n");
	scanf("%d", &i);
	pthread_exit((void *)pthread_self());
}

int main() {
	int err;
	void *status;
	unsigned long idt;
	int * arg = malloc(sizeof(*arg));

	mlockall(MCL_CURRENT|MCL_FUTURE);

	err = pthread_set_name_np(pthread_self(), "thread_pere");
	if (err) {
		printf("Error: main set_name_np\n");
		exit(EXIT_FAILURE);
	}
	
	err = pthread_attr_init(&attr);
	if (err) {
		printf("erreur pthread_attr_init\n");
		exit(EXIT_FAILURE);
	}

	err = pthread_create(&tid, &attr, (void *) thread_fils, arg); // Argument passé dans arg
	if (err) {
		printf("erreur create\n");
		exit (EXIT_FAILURE);
	}
	
	// err = pthread_join(tid, &status);
	// if (err) {
	// 	printf("erreur join\n");
	// 	exit (EXIT_FAILURE);
	// }
}
