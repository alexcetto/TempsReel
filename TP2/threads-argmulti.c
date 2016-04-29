#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

pthread_t tid;
pthread_attr_t attr;

typedef struct thread_args {
  int count;
  int * args;
} thread_args;

void thread_fils(thread_args *foo) {
  // printf("je suis le thread fils. \n\tPassed argument 1 : %d. \n\tPassed argument 2 : %d. \n\tMon pid est %d. \n\tMon tid est %u\n\tJe vais dormir 3s...\n",
  //  (int)foo->args[0], (int)foo->args[1], getpid(),(int)pthread_self());
  sleep(3);
  pthread_exit((void *)pthread_self());
}

int main() {
  int err;
  void *status;
  unsigned long idt;

  err = pthread_attr_init(&attr);
  if (err) {
      printf("erreur pthread_attr_init\n");
      exit(EXIT_FAILURE);
  }
  
  thread_args multiargs;
  multiargs.count = 2;
  multiargs.args = malloc(multiargs.count*sizeof(int));
  multiargs.args[0] = 42;
  multiargs.args[1] = 45;

  err = pthread_create(&tid, &attr, (void *) thread_fils, &multiargs);
  if (err) {
  printf("erreur create\n");
  exit (EXIT_FAILURE);
  }
  //------------ Detach process
  /*err = pthread_detach(tid);
  if (err) {
  printf("erreur detach\n");
  exit (EXIT_FAILURE);
  }*/
  //-------------------------------
  // printf("Je suis le pere. \n\tMon pid est %d \n\tMon tid est %u. \n\tJ'ai cree le thread fils de tid %u\n",
  //         getpid(), (int)pthread_self(),tid);
  err = pthread_join(tid, &status);
  if (err) {
    printf("erreur join\n");
    exit (EXIT_FAILURE);
  }
  idt = (unsigned long) status;
  // printf("Le pere a synchronise sur la fin du fils (tid %u)  \n\tstatus recu : %u\n",tid, idt);

}