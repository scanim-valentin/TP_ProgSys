#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

#define SEM_KEY 420
#define SHM_KEY 421
#define NB_ECHANGES 10

int main (int argc, char **argv){
    
  
  int sem,PID,mode;
  if(argc != 2){
        printf("Args : 1 pour le 1er processus / 0 pour un processus secondaire\n");
        exit(-1);
  }
  mode = atoi(argv[1]);
  struct sembuf buf;

   /* creation ou attachement du semaphore */
  printf("%d : Creation ou attachement du semaphore\n",PID);
  if(-1 == (sem=semget((key_t) SEM_KEY, 1, 0666)) ){
    printf("%d : pb creation du semaphore de production. Creation du semaphore\n",PID);
    if(-1 == (sem=semget((key_t) SEM_KEY, 1, 0666 | IPC_CREAT)) ){
        printf("%d : Echec de la creation du semaphore\n",PID);
        exit(-1);
    }
      /* initialisation du semaphore*/
    printf("%d : initialisation du semaphore\n",PID);
    if (-1 == semctl(sem, 0, SETVAL, 1) ){
      printf("%d : pb initialisation semaphore",PID);
      exit(-1);
    }
  
  }

  /* initialisation commune du buffer pour les operations sur le semaphore */
  printf("%d : initialisation commune du buffer pour les operations sur le semaphore\n",PID);
  buf.sem_num = 0;
  buf.sem_flg = 0;
  PID = getpid();
  
  //Partie création et initialisation SHM
  printf("%d : Partie création et initialisation SHM\n",PID);
  int *shm= NULL; /* pointeur sur la memoire */
  int shmid = 0; /* identificateur de la SHM */
  
  //On essaye d'obtenir la SHM associée à la clé 
  if (-1 == (shmid=shmget((key_t) SHM_KEY, 4, 0666))){
    printf("%d, pb obtention shm, création..\n",PID);
    printf("%d : Partie création SHM\n",PID);
    if (-1 ==(shmid=shmget((key_t) SHM_KEY, 4, 0666| IPC_CREAT))){ //Si la SHM n'existe pas on la crée
        printf("%d, pb creation shm\n",PID);
        exit(-1);
    }
    //Attachement & initialisation
    printf("%d : attachement de la shm\n",PID);
    if(!(shm=(int*)shmat(shmid, 0, 0))){
        printf("%d : pb d'attachement \n",PID); 
        exit(-1);
    }
    printf("%d : initialisation de la shm\n",PID);
    *shm=0;
  } else {
    printf("%d : attachement de la shm\n",PID);
    if(!(shm=(int*)shmat(shmid, 0, 0))){ //Si la SHM existe déjà, alors on l'attache (sans l'initialiser)
        printf("%d : pb d'attachement \n",PID); 
        exit(-1);
    }
  }
  
  printf("%d : SHM = %d\n",PID,*shm);   
   /* attachement de la shm */

  while(*shm< NB_ECHANGES){
      //P(S)
      //printf("instant 0 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
      buf.sem_op = -1;
      if (semop(sem, &buf, 1)){
	    printf ("%d : pb attente\n",PID);
	    //exit(-1);
      }
      //printf("instant 1 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
      
      //Zone critique
      (*shm)++;
      ////
      printf("%d : SHM = %d\n",PID,*shm);
      sleep(1);
      //V(S)
      buf.sem_op = 1;
      if (semop(sem, &buf, 1)){
	    printf ("%d : pb liberation\n",PID);
	    //exit(-1);
        
      }     
      //printf("instant 2 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
    }
    
    if(mode){
        printf("%d : liberation semaphore\n",PID);
        //destruction semaphore
        if(semctl(sem,0,IPC_RMID,0)){
        printf("%d : pb liberation semaphore ou semaphore deja detache\n",PID);
        }
        printf("%d : detache la SHM\n",PID);
        /* detache la SHM */
        if (shmdt(shm)){
            printf("%d : pb pour detache la SHM ou SHM deja detache\n",PID);
        }
        printf("%d : destruction de la SHM\n",PID);
        if (shmctl(shmid,IPC_RMID,0)){
        printf("%d : pb destruction SHM ou SHM deja detruite\n",PID);
        }
    }
    printf("%d : fin de tout\n",PID);
  return 0;
}


