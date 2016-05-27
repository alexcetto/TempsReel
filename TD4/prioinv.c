/* Demo for priority inversion phenomenon with mutexes
 * Version 6.3, May 13 2016 
 * (c) F.Touchard 2011-2016
 */

#define _GNU_SOURCE // Nécessaire pour sched.h
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <getopt.h>

#define CAPACITY 10000000

int prio_inherit=0;
int launch_t2=0;
int prtlog=0;

int nbsigxcpu=0;
cpu_set_t cpuset;


struct dspinfo_t {
  int action;
  int thread_nb;
  int cpu;
  time_t h_sec;
  time_t h_nsec;
};

struct dspinfo_t info[5000];
int nb_info = 0;

pthread_t tid_ordo, tid1, tid2, tid3;
pthread_attr_t attr;
struct sched_param sp;
int iaw[3];

pthread_mutexattr_t mut_attr;
pthread_mutex_t mutex;
struct itimerspec delay_timer1, delay_timer2, delay_timer3;
timer_t tim1, tim2, tim3;
  
struct timespec start_time, start_T1, start_T2, start_T3, resp_T1, resp_T2, resp_T3;

sem_t sem1, sem2, sem3, sem_o;



// Fonction permettant de faire un log du programme.
void dsp_info(int nb, int act) {

  if (!prtlog) return;

  struct timespec now;
  time_t sec, nsec;

  // Calcule le temps d'exécution d'un thread
  if (clock_gettime(CLOCK_REALTIME, &now) != 0) {
    perror("dsp_info clock_gettime");
    exit(1);
  }
  sec = now.tv_sec - start_time.tv_sec;
  nsec = now.tv_nsec - start_time.tv_nsec;
  if (nsec < 0) {
    nsec = nsec +1000000000;
    sec = sec-1;
  }
 
  // Sauvegarde dans la structure attitrée au thread
  info[nb_info].thread_nb = nb;
  info[nb_info].cpu = sched_getcpu();
  info[nb_info].action = act;
  info[nb_info].h_sec = sec;
  info[nb_info].h_nsec = nsec;
  nb_info++;
}

//***************************************************************************************
// Réalise une tâche d'une capacité de nb * CAPACITY
// La tâche consiste simplement à compter.
void active_wait(int nb) {
  int j, k, km, tid;
  
  if (pthread_equal(pthread_self(), tid1) != 0) {
    tid = 0;
  } else if (pthread_equal(pthread_self(), tid2) != 0) {
    tid = 1;
  } else if (pthread_equal(pthread_self(), tid3) != 0) {
    tid = 2;
  }

  km = nb*CAPACITY;

  for (j=0; j<10; j++) {
    for (k=0; k<km;k++) {
       iaw[tid]++;
    }
  }

}

//***************************************************************************************
// Le thread attend que le sémaphore soit libre pour réaliser ses actions
// Lorsqu'il est libre il bloque le mutex et réalise sa tâche
// Enfin il libère le mutex et réalise encore une tâche qui ne demande pas de mutex

void * thread1(void *foo) {
  struct timespec now;
  
  pthread_setname_np(pthread_self(), "thread1");
  
  sem_wait(&sem1);

  dsp_info(1, 0);

  dsp_info(1,2);
  if (pthread_mutex_lock(&mutex)) {
    perror ("thread1 mutex lock");
    exit(1);
  }

  dsp_info(1, 3);
  active_wait(5);
  
  dsp_info(1, 4);
  if (pthread_mutex_unlock(&mutex)) {
    perror ("thread1 mutex unlock");
    exit(1);
  }

  active_wait(1);
  
  dsp_info(1, 1);
  clock_gettime(CLOCK_REALTIME, &now);
  resp_T1.tv_sec = now.tv_sec - start_time.tv_sec;
  resp_T1.tv_nsec = now.tv_nsec - start_time. tv_nsec;
  if (resp_T1.tv_nsec < 0) {
    resp_T1.tv_nsec = resp_T1.tv_nsec+1000000000;
    resp_T1.tv_sec = resp_T1.tv_sec -1;
  }
  pthread_exit(NULL);
  
}


//***************************************************************************************
// Le thread2 attend la libération du sémaphore pour se lancer puis exécute une tâche.
void *thread2(void *foo) {
  struct timespec now;

  pthread_setname_np(pthread_self(), "thread2");
  
  sem_wait(&sem2);
  
  dsp_info(2, 0);
 
  active_wait(3);
  
  dsp_info(2, 1);
  clock_gettime(CLOCK_REALTIME, &now);
  resp_T2.tv_sec = now.tv_sec - start_time.tv_sec;
  resp_T2.tv_nsec = now.tv_nsec - start_time. tv_nsec;
  if (resp_T2.tv_nsec < 0) {
    resp_T2.tv_nsec = resp_T2.tv_nsec+1000000000;
    resp_T2.tv_sec = resp_T2.tv_sec -1;
  }
  pthread_exit(NULL);
}


//***************************************************************************************
// Même chose que le thread1
void *thread3(void *foo) {
  struct timespec now;

  pthread_setname_np(pthread_self(), "thread3");
  
  sem_wait(&sem3);
  
  dsp_info(3, 0);
  // Effectue une permière partie de la tâche ici
  active_wait(1);
  
  dsp_info(3, 2);
  if (pthread_mutex_lock(&mutex)) {
    perror ("thread3 mutex lock");
    exit(1);
  }

  dsp_info(3, 3);
  // Deuxième partie de la tâche en exclusion mutuelle
  active_wait(2);

  dsp_info(3, 4);

  if (pthread_mutex_unlock(&mutex)) {
    perror ("thread3 mutex unlock");
    exit(1);
  }
  // Termine la tâche normalement
  active_wait(1);
  dsp_info(3, 1);

  clock_gettime(CLOCK_REALTIME, &now);
  resp_T3.tv_sec = now.tv_sec - start_time.tv_sec;
  resp_T3.tv_nsec = now.tv_nsec - start_time. tv_nsec;
  if (resp_T3.tv_nsec < 0) {
    resp_T3.tv_nsec = resp_T3.tv_nsec+1000000000;
    resp_T3.tv_sec = resp_T3.tv_sec -1;
  }

  pthread_exit(NULL);
    
}

//*********************************************************************************
// Thread s'occupant de l'odonnancement
void *thread_ordo(void *foo) {
 int err;
  struct timespec delay;
  
  pthread_setname_np(pthread_self(), "ordo");

  // Défini la priorité par défaut
  sp.sched_priority = 1;
  err = pthread_attr_setschedparam(&attr, &sp);
  if (err) {
    printf("erreur main setscherparam\n");
    exit(EXIT_FAILURE);
  }

  // Crée le thread 1 avec une priorité de 1
  if (pthread_create(&tid1, &attr, &thread1, NULL)) {
    perror("pthread_create thread1");
    exit(1);
  }
  // Affecte le thread à un coeur CPU pour une meilleure consistance du RT
  pthread_setaffinity_np(tid1, sizeof(cpu_set_t), & cpuset);

  sp.sched_priority = 2;
  err = pthread_attr_setschedparam(&attr, &sp);
  if (err) {
    printf("erreur main setschedparam\n");
    exit(EXIT_FAILURE);
  }

   // Crée le thread 2 si besoin avec une priorité de 2
  if (launch_t2) {
    if (pthread_create(&tid2, &attr, &thread2, NULL)) {
      perror("pthread_create threadé");
      exit(1);
    }
    pthread_setaffinity_np(tid2, sizeof(cpu_set_t), & cpuset);
  }
  

  // Enfin thread 3 avec une priorité de 3
  sp.sched_priority = 3;
  err = pthread_attr_setschedparam(&attr, &sp);
  if (err) {
    printf("erreur main setscherparam\n");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&tid3, &attr, &thread3, NULL)) {
    perror("pthread_create thread3");
    exit(1);
  }
  pthread_setaffinity_np(tid3, sizeof(cpu_set_t), & cpuset);
  

  delay.tv_sec = 0;
  delay.tv_nsec = 100000000;
  err = nanosleep(&delay, NULL);
  if (err) {
    perror("nanosleep pour sem1");
    exit(EXIT_FAILURE);
  }
  // Incrémente le sémaphore 1, 2, 3 successivement
  err = sem_post(&sem1);
  if (err) {
    perror("post sem1");
    exit(EXIT_FAILURE);
  }
  
  delay.tv_sec = 0;
  delay.tv_nsec = 500000000;
  err = nanosleep(&delay, NULL);
  if (err) {
    perror("nanosleep pour sem2");
    exit(EXIT_FAILURE);
  }
  err = sem_post(&sem2);
  if (err) {
    perror("post sem2");
    exit(EXIT_FAILURE);
  }
  
  delay.tv_sec = 0;
  delay.tv_nsec = 500000000;
  err = nanosleep(&delay, NULL);
  if (err) {
    perror("nanosleep pour sem3");
    exit(EXIT_FAILURE);
  }

err = sem_post(&sem3);
  if (err) {
    perror("post sem2");
    exit(EXIT_FAILURE);
  }

  // Attend que les threads se termine avant de se terminer
  pthread_join(tid1, NULL);
  if (launch_t2) pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);
  
  pthread_exit(NULL);

}
//***************************************************************************************
void got_sig(int num) {
  dsp_info(0,5);
}
//***************************************************************************************

int main(int argc, char **argv) {
  int err, n;
  int c;
  
  char *info_txt;

/* au cas où le watchdog enverrait SIGXCPU */
  struct sigaction sa;
  sa.sa_handler = got_sig;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGXCPU, &sa, NULL);


  while ((c = getopt (argc, argv, "ptlh")) != -1)
    switch (c)
      {
      case 'p':
        prio_inherit = 1;
        break;
      case 't':
        launch_t2 = 1;
        break;
      case 'l':
        prtlog = 1;
        break;
      case 'h':
	printf("Recognized options :\n\t-p -> invokes priority inheritance\n\t-t -> launches thread T2\n\t-l -> produces a log\n");
	exit(0);
      case '?':
	if (isprint (optopt))
     fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
     fprintf (stderr,
         "Unknown option character `\\x%x'.\n",
         optopt);
        return 1;
      default:
        abort ();
      }

  
  if (prio_inherit) {
    printf("Priority inheritance for mutexes\n");
  } else {
    printf("No priority inheritance for mutexes\n");
  }
  if (launch_t2) {
    printf("T2 thread will be launched\n");
  } else {
    printf("no T2 thread\n");
  }
  if (prtlog) {
    printf("Log will be displayed\n");
  } else {
    printf("no log\n");
  }

  if (clock_gettime(CLOCK_REALTIME, &start_time) != 0) {
    perror("clock_gettime");
    exit(1);
  }
  // Verrouille l'espace mémoire 
  mlockall(MCL_CURRENT|MCL_FUTURE);

// all threads on CPU0
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);
  
  iaw[0] = 0;
  iaw[1] = 0;
  iaw[2] = 0;
  // initialise les propriétés du mutex
  if (pthread_mutexattr_init(&mut_attr)) {
    perror("mutexattr_init");
    exit(1);
  }
  // Puis définit l'héritage de priorité en fonction des options
  if (prio_inherit) {
    if (pthread_mutexattr_setprotocol(&mut_attr, PTHREAD_PRIO_INHERIT)){
  perror("PRIO_INHERIT attribute");
  exit(1);
    }
  }
  
  if (pthread_mutex_init(&mutex, &mut_attr)) {
    perror("mutex_init");
    exit(1);
  }
// initiatilisation des sémaphores
  err = sem_init(&sem1, 0, 0);
  if (err) {
    perror("sem_init 1");
    exit(1);
  }
  
  err = sem_init(&sem2, 0, 0);
  if (err) {
    perror("sem_init 1");
    exit(1);
  }
   
  err = sem_init(&sem3, 0, 0);
  if (err) {
    perror("sem_init 3");
    exit(1);
  }
  
  if (pthread_attr_init(&attr)) {
    perror("pthread_attr_init");
  }
  // On précise qu'on va modifier le scheduling
  err = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  if (err) {
    printf("erreur main setinheritsched\n");
    exit(EXIT_FAILURE);
  }
  // Scheduling FIFO
  err = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  if (err) {
     printf("erreur main setschedpolicy\n");
     exit(EXIT_FAILURE);
  }

  sp.sched_priority = 4;
  err = pthread_attr_setschedparam(&attr, &sp);
  if (err) {
    printf("erreur main setscherparam\n");
    exit(EXIT_FAILURE);
  }
  // L'ordonnanceur a une priorité de 4
  if (pthread_create(&tid_ordo, &attr, thread_ordo, NULL)) {
    perror("pthread_create thread_ordo");
    exit(EXIT_FAILURE);
  }
  // Sur le CPU0
  pthread_setaffinity_np(tid_ordo, sizeof(cpu_set_t), & cpuset);

  pthread_join(tid_ordo, NULL);
  
  // Affichage des informations de log
  for (n=0; n< nb_info; n++) {

    switch (info[n].action) {
      case 0 :
      	info_txt = "starts";
      	break;
      case 1 : 
      	info_txt = "stops";
      	break;
      case 2 : 
      	info_txt = "requests lock";
      	break;
      case 3 : 
      	info_txt = "gets lock";
      	break;
      case 4 : 
      	info_txt= "releases lock";
      	break;
      case 5 : 
      	info_txt= "SIGXCPU";
      	break;
    }
    
    printf("thread %d on CPU %d: %s at %d.%03ds\n", 
	   info[n].thread_nb, info[n].cpu, info_txt, (int)info[n].h_sec, (int)info[n].h_nsec/1000000);
  }

  printf("response time for T1: %d.%03ds  (C :%d) \n",
	 (int)resp_T1.tv_sec, (int)resp_T1.tv_nsec/1000000,iaw[0]);
  if (launch_t2) printf("response time for T2: %d.%03ds  (C :%d) \n",
    (int)resp_T2.tv_sec, (int)resp_T2.tv_nsec/1000000, iaw[1]);
  printf("response time for T3: %d.%03ds  (C :%d) \n",
	 (int)resp_T3.tv_sec, (int)resp_T3.tv_nsec/1000000, iaw[2]);
  exit(0);
}
