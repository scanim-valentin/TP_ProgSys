#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#define SEM_KEY 420

int main ()
{
  int sem;
  struct sembuf buf;

   /* creation ou attachement du semaphore */
  
  if(-1 == (sem=semget((key_t) SEM_KEY, 1, 0666)) ){
    printf("pb creation du semaphore de production. Creation du semaphore\n");
    if(-1 == (sem=semget((key_t) SEM_KEY, 1, 0666 | IPC_CREAT)) ){
        printf("Echec de la creation du semaphore\n");
        exit(-1);
    }
      /* initialisation du semaphore*/
    if (-1 == semctl(sem, 0, SETVAL, 1) ){
      printf("pb initialisation semaphore");
      exit(-1);
    }
  
  }
  

  /* initialisation commune du buffer pour les operations sur le semaphore */
  buf.sem_num = 0;
  buf.sem_flg = 0;
  char input[100];
  input[99] = '\0';
  input[0] = '\0';
  int PID = getpid();
  while(1){
      //P(S)
      printf("instant 0 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
      buf.sem_op = -1;
      if (semop(sem, &buf, 1)){
	    printf ("PID %d : pb attente\n",PID);
	    exit(-1);
      }
      printf("instant 1 : Semaphore = %d\n",semctl(sem, 0, GETVAL));

      /* attente d'une chaine de caracteres de l'utilisateur */
      printf("Entrez une chaine : ");
      //scanf("%s",input);
      fgets(input, 99, stdin);
      strtok(input, "\n");           /* Suppression du '\n' */
      printf("\nPID %d : Chaine : %s\n",PID,input);
      
      //V(S)
      buf.sem_op = 1;
      if (semop(sem, &buf, 1)){
	    printf ("PID %d : pb liberation\n",PID);
	    exit(-1);
        
      }     
      printf("instant 2 : Semaphore = %d\n",semctl(sem, 0, GETVAL));
    }
  return 0;
}
