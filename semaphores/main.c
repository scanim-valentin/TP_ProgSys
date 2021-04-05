#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

int main (int argc, char * argv[])
{

  int key;
  int tache;
  int sem;
  int init = 1;
  struct sembuf buf;
  int i;

  if( argc != 2){
    printf("pb argument: numero cle \n");
    exit(-1);
  }
  key = atoi(argv[1]);
  
   /* creation ou attachement du semaphore */
  sem=semget((key_t) key, 1, 0666| IPC_CREAT);
  if(sem == -1){
    printf("pb creation du semaphore de production\n");
    exit(-1);
  }
  
  /* initialisation du semaphore par le consommateur */
  if (semctl (sem, 0, SETVAL, init)== -1){
      printf("pb initialisation semaphore de production");
      exit(-1);
  }
  
  /* initialisation commune du buffer pour les operations sur le semaphore */
  buf.sem_num = 0;
  buf.sem_flg = 0;
  char input[100];
  input[99] = '\0';
  input[0] = '\0';
  int PID = getpid();
  while(1){
      
      buf.sem_op = -1;
      if (semop(sem, &buf, 1)){
	    printf ("PID %d : pb attente\n",PID);
	    exit(-1);
      }
      /* attente d'une chaine de caracteres de l'utilisateur */
      scanf("%s",input);
      printf("\nPID %d : Chaine : %s\n",PID,input);
        
      
      
      buf.sem_op = 1;
      sleep(10);
      if (semop(sem, &buf, 1)){
	    printf ("PID %d : pb liberation\n",PID);
	    exit(-1);
        
      }     
    }
  return 0;
}
