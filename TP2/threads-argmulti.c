#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

pthread_t tid;
pthread_attr_t attr;

// On utilise ici une structure pour stocker les arguments, 
// puisqu'il n'est possible de passer qu'un seul argument avec pthread_create
typedef struct args_str {
	short no; 	// Nombre d'arguments
	int * args;	// Contient les arguments
} args_str;


void thread_fils(args_str *foo) {
	int i;
	int top = foo->no;

	printf("Arguments reçus : \n");
	for(i=0; i<top; i++)
		printf("Argument : %d\n", foo->args[i]);
	sleep(3);
	pthread_exit((void *)pthread_self());
}


int main() {
	int err, i;
	void *status;
	unsigned long idt;

	err = pthread_attr_init(&attr);
	if (err) {
		printf("erreur pthread_attr_init\n");
		exit(EXIT_FAILURE);
	}

	args_str args;
	args.no = 2;
	args.args = malloc(args.no*sizeof(int));
	args.args[0] = 42;
	args.args[1] = 67;

	printf("Arguments passés : \n");
	for(i=0; i<args.no; i++)
		printf("Argument : %d\n", args.args[i]);
	printf("\n");

	err = pthread_create(&tid, &attr, (void *) thread_fils, &args);
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