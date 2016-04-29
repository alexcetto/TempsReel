#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

pthread_t tid;
pthread_attr_t attr;

void thread_fils(void *foo) {
  /*printf("je suis le thread fils. \n\tMon pid est %d. \n\tMon tid est %u\n\tJe vais dormir 3s...\n",
	 getpid(),(int)pthread_self());
  */

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
  err = pthread_create(&tid, &attr, (void *) thread_fils, arg);
  if (err) {
	printf("erreur create\n");
	exit (EXIT_FAILURE);
  }
  /*printf("Je suis le pere. \n\tMon pid est %d \n\tMon tid est %u. \n\tJ'ai cree le thread fils de tid %u\n",
          getpid(), (int)pthread_self(),tid);
  */
  err = pthread_join(tid, &status);
  if (err) {
    printf("erreur join\n");
    exit (EXIT_FAILURE);
  }
  idt = (unsigned long) status;
  //printf("Le pere a synchronise sur la fin du fils (tid %u)  \n\tstatus recu : %u\n",tid, idt);

}
