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

#define SEM_CREA_KEY 420 //Clé de sem_crea

#define SHM_PIDS_KEY 421 //Clé des shm
#define SHM_CAMP_KEY 422

typedef struct{ //Construction d'une structure IPC permettant de transmettre la Memoire Partage et les Semaphores aux enfants
 int sem_crea_mage; //Semaphore qui représente le nombre de mages qu'il reste a créer
 struct sembuf buf; //Buffer associé à la sémaphore précédente
 
 int nb_mage; //Nombre de mages 
 
 int* shm_PIDs; //Un tableau partagé des PID de chacun (chaque process peut connaitre toute le monde)
 int shmid_PIDs; //L'ID de la SHM associée  
                //Note : Un mage associé ainsi à un index
 char* shm_Camp; //Un tableau partagé de la liste du camp de chaque mage 
 int shmid_Camp; //L'ID de la SHM associée  

 int PID; //PID du process 
 int index;
 int vie; //Vie du mage
}ipc;

ipc * init_ipc(int nb_mage){ //Fonction d'initialisation de la structure ipc
    ipc * R = (ipc*)malloc(sizeof(ipc));
    R->PID = getpid();
    R->nb_mage = nb_mage;
    
    printf("%d : Creation ou attachement du semaphore\n",R->PID); //On prends le semaphore si il existe deja
    if(-1 == (R->sem_crea_mage=semget((key_t) SEM_CREA_KEY, 1, 0666)) ){
        printf("%d : pb creation du semaphore de production. Creation du semaphore\n",R->PID);
        if(-1 == (R->sem_crea_mage=semget((key_t) SEM_CREA_KEY, 1, 0666 | IPC_CREAT)) ){ //Sinon on le crée
            printf("%d : Echec de la creation du semaphore\n",R->PID);
            exit(-1);
        }
    }
        /* initialisation du semaphore*/
    printf("%d : initialisation du semaphore\n",R->PID); //Dans tous les cas, on initie ce semaphore à nb_mage-1
    if (-1 == semctl(R->sem_crea_mage, 0, SETVAL, nb_mage) ){
        printf("%d : pb initialisation semaphore",R->PID);
        exit(-1);
    } 
    
    /* initialisation commune du buffer pour les operations sur le semaphore */
  printf("%d : initialisation commune du buffer pour les operations sur le semaphore\n",R->PID);
  R->buf.sem_num = 0;
  R->buf.sem_flg = 0;
  
  //Partie création et initialisation SHM shm_PIDs
  printf("%d : Partie création et initialisation SHM\n",R->PID);
  R->shm_PIDs = NULL; /* pointeur sur la memoire */
  R->shmid_PIDs = 0; /* identificateur de la SHM */
  
  //On essaye d'obtenir la SHM associée à la clé 
  if (-1 == (R->shmid_PIDs=shmget((key_t) SHM_PIDS_KEY, 4*nb_mage, 0666))){ 
    printf("%d, pb obtention shm pids, création..\n",R->PID);
    printf("%d : Partie création SHM pids\n",R->PID);
    if (-1 ==(R->shmid_PIDs=shmget((key_t) SHM_PIDS_KEY, 4*nb_mage, 0666| IPC_CREAT))){ //Si la SHM n'existe pas on la crée
        printf("%d, pb creation shm pids\n",R->PID);
        exit(-1);
    }
  }
  //Attachement & initialisation
  printf("%d : attachement de la shm pids\n",R->PID);
  if(!(R->shm_PIDs=(int*)shmat(R->shmid_PIDs, 0, 0))){
    printf("%d : pb d'attachement pids\n",R->PID); 
    exit(-1);
  }
  printf("%d : initialisation  à 0 de la shm pids\n",R->PID);
  
  printf("%d : Etat de SHM_PIDS : ",R->PID);   
  for(int i = 0; i<nb_mage;i++){
      (R->shm_PIDs)[i] = 0;
      printf("[%d]",(R->shm_PIDs)[i]);   
  }
  printf("\n");
  
  
  //Partie création et initialisation SHM shm_CAMP
  printf("%d : Partie création et initialisation SHM_CAMP\n",R->PID);
  R->shm_Camp = NULL; /* pointeur sur la memoire */
  R->shmid_Camp = 0; /* identificateur de la SHM */
  
  //On essaye d'obtenir la SHM associée à la clé 
  if (-1 == (R->shmid_Camp=shmget((key_t) SHM_CAMP_KEY, nb_mage, 0666))){  //Taille  = nb_mages
    printf("%d, pb obtention shm camps, création..\n",R->PID);
    printf("%d : Partie création SHM camps\n",R->PID);
    if (-1 ==(R->shmid_Camp=shmget((key_t) SHM_CAMP_KEY, nb_mage, 0666| IPC_CREAT))){ //Si la SHM n'existe pas on la crée
        printf("%d, pb creation shm camps\n",R->PID);
        exit(-1);
    }
  }
  //Attachement & initialisation
  printf("%d : attachement de la shm camps\n",R->PID);
  if(!(R->shm_Camp=(char*)shmat(R->shmid_Camp, 0, 0))){
    printf("%d : pb d'attachement camps\n",R->PID); 
    exit(-1);
  }
  printf("%d : initialisation  à 0 de la shm camps\n",R->PID);
  
  printf("%d : Etat de SHM_CAMP: ",R->PID);   
  for(int i = 0; i<nb_mage;i++){
      (R->shm_Camp)[i] = 0;
      printf("[%d]",(R->shm_Camp)[i]);   
  }
  printf("\n");
  
  printf("Fin de l'initialisation de ipc\n");
  return R;  
}

void destroy_ipc(ipc * IPC){
    //destruction semaphore
    if(semctl(IPC->sem_crea_mage,0,IPC_RMID,0)){
            printf("%d : pb liberation semaphore crea_mage ou semaphore deja detache\n",IPC->PID);
            exit(-1);
    }
    printf("%d : detache la SHM crea_mage\n",IPC->PID);
    
    /* detache les SHM  */
    if (shmdt(IPC->shm_Camp)){
        printf("%d : pb pour detache la SHM Camp ou SHM deja detache\n",IPC->PID);
        exit(-1);
    }
    printf("%d : destruction de la SHM  Camp \n",IPC->PID);
    if (shmctl(IPC->shmid_Camp,IPC_RMID,0)){
        printf("%d : pb destruction SHM Camp ou SHM deja detruite\n",IPC->PID);
        exit(-1);
    }
    
    if (shmdt(IPC->shm_PIDs)){
        printf("%d : pb pour detache la SHM PIDs ou SHM deja detache\n",IPC->PID);
        exit(-1);
    }
    printf("%d : destruction de la SHM  PIDs \n",IPC->PID);
    if (shmctl(IPC->shmid_PIDs,IPC_RMID,0)){
        printf("%d : pb destruction SHM PIDs ou SHM deja detruite\n",IPC->PID);
        exit(-1);
    }
    free(IPC);
    printf("%d : Structure IPC detruite\n",IPC->PID);
}


char EstBien(int PID){ //Somme les numéros composant PID pour déterminer si le process représente le bien ou le mal (pair ou impair)
    
    int S = 0;
    char string_PID[20];
    string_PID[19] = '\0';
    sprintf(string_PID,"%d",PID);
    for(int i = 0 ; string_PID[i] != '\0' ; i++)
        S += atoi(&string_PID[i]);
    return (0 == S%2);
}

int child(ipc * IPC){ //Appel récursif de cette fonction, limité par le semaphore sem_crea_mage
    int R=-1;
    printf("Processus enfant créé PID = %d\n",IPC->PID=getpid());
    
    //On prélève l'index de notre mage et décrémente notre sémaphore sem_crea 
    IPC->buf.sem_op = -1;
    if(-1 == (IPC->index = semctl(IPC->sem_crea_mage, 0, GETVAL)) ) { //On récupère la valeur du sémaphore qui sert à déterminer le rôle du processus de calcul
        printf("Enfant %d: Erreur lors de la recuperation de la valeur du semaphore", IPC->PID);
        exit(-1);
    }
      /* traitement de l'element */
    printf("Index %d assigne à %d\n",IPC->index,IPC->PID);
   
    //On mets un timer sur l'obtention du semaphore sem_crea_mage
    struct timespec timer;
    timer.tv_sec = 2; //2 secondes
    if (semtimedop(sem_crea_mage, &(IPC->buf), 1,&timer)){
        printf ("Enfant %d: Pb semop sem_crea\n",IPC->PID);
        exit(-1);
      }
    free(&timer);
    switch(fork()){ //On fork et on va rappeller récursivement cette fonction
        case(-1) :
            printf("Enfant %d: Echec du fork\n", IPC->PID);
            exit(-1);
                
        case(0) : //On est dans le petit-enfant
            if(-1 == child(IPC)){
                printf("Enfant %d: Echec lors de l'appel de child()\n", IPC->PID);
                exit(-1);
            }
            if(1 == child(IPC)){
                printf("Enfant %d: Tous les processus ont été généré\n", IPC->PID);
            }
            
            break;

        default : ;//On est dans l'enfant
            //bataille(IPC);
    }
    return 0;
}

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Utilisation : nb_mages\n");
        exit(-1);
    }
    ipc * IPC = init_ipc(atoi(argv[1]));
    printf("%d : Fork...\n",IPC->PID = getpid());
    switch(fork()){
        case -1 : //Le fork a echoué
            printf("Le fork a echoué\n",IPC->PID);
            exit(-1);
        case 0 : //Dans l'enfant
            
            ///////Machin les sorts
            break;
        default:
            printf("Debut combat pour : %d\n",IPC->PID);
            ///////Machin les sorts
                
            destroy_ipc(IPC);
            break;
    }
    
    return 0;
}
