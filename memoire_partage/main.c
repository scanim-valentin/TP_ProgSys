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
#define NB_ECHANGES 1000

int PID;

int proc ()
{
  int sem;
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
  int PID = getpid();
  
  //Partie création et initialisation SHM
  printf("%d : Partie création et initialisation SHM\n",PID);
  int *shm= NULL; /* pointeur sur la memoire */
  int shmid = 0; /* identificateur de la SHM */
  
  
  if (-1 == (shmid=shmget((key_t) SHM_KEY, 4, 0666))){
    printf("%d, pb obtention shm, création..\n",PID);
    printf("%d : Partie création SHM\n",PID);
    if (-1 ==(shmid=shmget((key_t) SHM_KEY, 4, 0666| IPC_CREAT))){ //Si SHM existe pas on la crée
        printf("%d, pb creation shm\n",PID);
        exit(-1);
    }
    printf("%d : Partie initialisation SHM\n",PID);
    *shm=5;
  }
      
   /* attachement de la shm */
  printf("%d : attachement de la shm\n",PID);
  if(!(shm=(int*)shmat(shmid, 0, 0))){
    printf("%d : pb d'attachement \n",PID);
    exit(-1);
  }
  for(int i = 0; i< NB_ECHANGES; i++){
      //P(S)
      //printf("instant 0 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
      buf.sem_op = -1;
      if (semop(sem, &buf, 1)){
	    printf ("%d : pb attente\n",PID);
	    exit(-1);
      }
      //printf("instant 1 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
      
      
      printf("%d : SHM = %d\n",PID,*shm);
      //V(S)
      buf.sem_op = 1;
      if (semop(sem, &buf, 1)){
	    printf ("%d : pb liberation\n",PID);
	    exit(-1);
        
      }     
      //printf("instant 2 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
    }
    printf("%d : liberation semaphore\n",PID);
    //destruction semaphore
    if(semctl(sem,0,IPC_RMID,0)){
      printf("%d : pb liberation semaphore\n",PID);
      exit(-1);
    }
    printf("%d : detache la SHM\n",PID);
    /* detache la SHM */
    if (shmdt(shm)){
        printf("%d : pb pour detache la SHM\n",PID);
        exit(-1);
    }
    printf("%d : destruction de la SHM\n",PID);
    if (shmctl(shmid,IPC_RMID,0)){
      printf("%d : pb destruction shm\n",PID);
      exit(-1);
    }
    printf("%d : fin de tout\n",PID);
  return 0;
}

int main(){
 switch(PID = fork()){
     case -1 : //Echec du fork
         printf("Echec du fork");
         exit(-1);
     
     case 0 : //On est dans l'enfant
        PID = getpid();
        printf("Enfant (moi) (P2) créé (PID = %d )\n", PID);
        proc();
        break;
     default: //On est dans le parent (PID = le PID de l'enfant)
         
        PID = getpid();
        printf("Parent (moi) (P1) (PID = %d )\n", PID);
        proc();
        break;
 }
 return 0;
}
