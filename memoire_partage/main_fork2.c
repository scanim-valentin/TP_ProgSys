#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

/* le temps */
#include <sys/time.h>

#define SEM_KEY 420
#define SHM_KEY 421
#define NB_ECHANGES 1000

typedef struct{
 int sem;
 struct sembuf buf;
 
 int* shm;
 int shmid;
 
 int PID;
}ipc;

ipc * init_ipc(){
    ipc * R = (ipc*)malloc(sizeof(ipc));
    R->PID = getpid();
    printf("%d : Creation ou attachement du semaphore\n",R->PID);
    if(-1 == (R->sem=semget((key_t) SEM_KEY, 1, 0666)) ){
        printf("%d : pb creation du semaphore de production. Creation du semaphore\n",R->PID);
        if(-1 == (R->sem=semget((key_t) SEM_KEY, 1, 0666 | IPC_CREAT)) ){
            printf("%d : Echec de la creation du semaphore\n",R->PID);
            exit(-1);
        }
    }
        /* initialisation du semaphore*/
    printf("%d : initialisation du semaphore\n",R->PID);
    if (-1 == semctl(R->sem, 0, SETVAL, 1) ){
        printf("%d : pb initialisation semaphore",R->PID);
        exit(-1);
    } 
    
    /* initialisation commune du buffer pour les operations sur le semaphore */
  printf("%d : initialisation commune du buffer pour les operations sur le semaphore\n",R->PID);
  R->buf.sem_num = 0;
  R->buf.sem_flg = 0;
  R->PID = getpid();
  
  //Partie création et initialisation SHM
  printf("%d : Partie création et initialisation SHM\n",R->PID);
  R->shm = NULL; /* pointeur sur la memoire */
  R->shmid = 0; /* identificateur de la SHM */
  
  //On essaye d'obtenir la SHM associée à la clé 
  if (-1 == (R->shmid=shmget((key_t) SHM_KEY, 4, 0666))){
    printf("%d, pb obtention shm, création..\n",R->PID);
    printf("%d : Partie création SHM\n",R->PID);
    if (-1 ==(R->shmid=shmget((key_t) SHM_KEY, 4, 0666| IPC_CREAT))){ //Si la SHM n'existe pas on la crée
        printf("%d, pb creation shm\n",R->PID);
        exit(-1);
    }
  }
  //Attachement & initialisation
  printf("%d : attachement de la shm\n",R->PID);
  if(!(R->shm=(int*)shmat(R->shmid, 0, 0))){
    printf("%d : pb d'attachement \n",R->PID); 
    exit(-1);
  }
  printf("%d : initialisation de la shm\n",R->PID);
  *(R->shm)=0;

  
  printf("%d : SHM = %d\n",R->PID,*(R->shm));   
   /* attachement de la shm */
  return R;  
}

void destroy_ipc(ipc * IPC){
    //destruction semaphore
    if(semctl(IPC->sem,0,IPC_RMID,0)){
            printf("%d : pb liberation semaphore ou semaphore deja detache\n",IPC->PID);
    }
    printf("%d : detache la SHM\n",IPC->PID);
    /* detache la SHM */
    if (shmdt(IPC->shm)){
        printf("%d : pb pour detache la SHM ou SHM deja detache\n",IPC->PID);
    }
    printf("%d : destruction de la SHM\n",IPC->PID);
    if (shmctl(IPC->shmid,IPC_RMID,0)){
        printf("%d : pb destruction SHM ou SHM deja detruite\n",IPC->PID);
    }
    printf("%d : Structure IPC detruite\n",IPC->PID);
}

int proc (ipc * IPC)
{ 

    while(*(IPC->shm)< NB_ECHANGES){
      //P(S)
      //printf("instant 0 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
      IPC->buf.sem_op = -1;
      if(semop(IPC->sem, &(IPC->buf), 1)){
	    printf ("%d : pb attente\n",IPC->PID);
	    exit(-1);
      }
      //printf("instant 1 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
      
      //Zone critique
      (*(IPC->shm))++;
      ////
      printf("%d : SHM = %d\n",IPC->PID,*(IPC->shm));
      //V(S)
      IPC->buf.sem_op = 1;
      if(semop(IPC->sem, &(IPC->buf), 1)){
	    printf ("%d : pb liberation\n",IPC->PID);
	    exit(-1);
        
      }     
      //printf("instant 2 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
    }
  return 0;
}

int main(){
 ipc* IPC = init_ipc();
 //Le temps
 unsigned long tf, td;
 struct timeval* restrict tp = (struct timeval*)malloc(sizeof(struct timeval));
 gettimeofday(tp,NULL);
 printf("Le temps : %lu\n",tp->tv_usec);
 printf("Appel de fork: %d\n",IPC->PID);
 switch(IPC->PID = fork()){
     case -1 : //Echec du fork
         printf("-1\n");
         printf("Echec du fork\n");
         exit(-1);
     
     case 0 : //On est dans l'enfant
        printf("0\n");
        IPC->PID = getpid();
        printf("Enfant (moi) (P2) créé (PID = %d )\n", IPC->PID);
        proc(IPC);
        break;
     default: //On est dans le parent (PID = le PID de l'enfant)
        printf("default\n");
        IPC->PID = getpid();
        printf("Parent (moi) (P1) (PID = %d )\n", IPC->PID);
        //Le temps
        gettimeofday(tp,NULL);
        td = tp->tv_usec;
        proc(IPC);
        //Le temps
        gettimeofday(tp,NULL);
        tf = tp->tv_usec;
        float delta = ((float)tf-(float)td);
        float d = NB_ECHANGES*64./delta;
        printf("td = %lu, tf = %lu, delta = %f, Debit: %f Mb/s\n",td,tf,delta,d);
        printf("%d : liberation semaphore\n",IPC->PID);
        destroy_ipc(IPC);
        break;
 }
 return 0;
}
