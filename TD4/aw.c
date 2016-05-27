#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sched.h>

int main(int argc, char * argv[]) {
 
  int nbw, i, j=0;
  struct timespec start, stop, duration;
  struct sched_param param;
  
  if (argc != 2) {
    printf("syntax : aw nb_cycles\n");
    exit(EXIT_FAILURE);
  }
  nbw = atoi(argv[1]);
  
  param.sched_priority = 1;
  if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
    perror("sched_setscheduler");
    exit(EXIT_FAILURE);
  }
  
  if (clock_gettime(CLOCK_REALTIME, &start) != 0) {
    perror("clock_gettime start");
    exit(EXIT_FAILURE);
  }
  for (i=0; i<nbw; i++) j++;
  if (clock_gettime(CLOCK_REALTIME, &stop) != 0) {
    perror("clock_gettime start");
    exit(EXIT_FAILURE);
  }
  duration.tv_sec = stop.tv_sec - start.tv_sec;
  duration.tv_nsec = stop.tv_nsec - start.tv_nsec;
  if (duration.tv_nsec < 0) {
    duration.tv_nsec = duration.tv_nsec+1000000000;
    duration.tv_sec--;
  }
  printf("nb of cycles : %d, duration %ds%9d\n", nbw, (int)duration.tv_sec, (int)duration.tv_nsec);
  
  exit(0);
}