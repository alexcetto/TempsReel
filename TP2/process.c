#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int status;
pid_t pidPere, pidFils;

      
int process_fils() {
     printf("Je suis le process fils, mon pid est %d, le pid de mon pere est %d\n", 
	     getpid(), getppid());
     sleep (3);
     exit(0);
}

int process_pere() {
     waitpid(pidFils,&status,0); 
     printf("je suis le pere, j'ai recu fin du fils pid %d\n",pidFils);
     exit(0);
}

int main() {
    pidPere = getpid();
    printf("je suis le pere, mon pid est %d\n", pidPere);
    switch (pidFils=fork()) {
    case -1: 
      perror("fork");
      exit(2);
      break;
    case 0:
      process_fils();
      break;
    default:
      process_pere();
      break;
    }
}

