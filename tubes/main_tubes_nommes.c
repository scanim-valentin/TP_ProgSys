/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* pour les entrées/sorties */
#include <stdio.h>
/*manipulation des strings*/
#include <string.h>
/* pour la gestion des erreurs */
#include <errno.h>
/* pour la gestion des tubes nommes */
#include <fcntl.h>
/* le temps */
#include <sys/time.h>
/*????*/
#include <sys/stat.h>

#define FILE_PATH "/tmp/pipe_papa"
#define NB_ECHANGES 1000

int main(){
    //Calcul du debit
    /*struct timeval* restrict tp = (struct timeval*)malloc(sizeof(struct timeval));
    gettimeofday(tp,NULL);
    printf("Le temps : %lu\n",tp->tv_usec );
    unsigned long tf, td;
    */
    
    
	char token = 'a';
    if(-1 == mkfifo(FILE_PATH,S_IRUSR | S_IWUSR | S_IRGRP)){ //Création du pipe
    	printf("Echec lors de la creation du pipe du pere ( mkfifo )\n");
        exit(-1);
    }
    int pipe_papa;
    int pid = fork(); // Retourne 0 pour le fils, le PID du fils pour le père, -1 si erreur
    switch(pid){
            
            case(-1) :
                printf("Echec du fork\n");
                exit(1);
                
            case(0) : //On est dans le fils
                if(-1 == (pid = getpid()) ){
                    printf("--- Echec lors de l'appel de getpid()\n");
                    exit(1);
                }
                
                printf("--- On est dans le fils, pid = ");
                printf("%d\n",pid);
                pipe_papa = open(FILE_PATH,O_WRONLY);
                
                if (-1 == pipe_papa)
                        printf("pb open fils\n");
                
                if (1 != write(pipe_papa,&token,1)){
                    printf("pb write fils\n");
                    exit(-1);
                }
                break;

            default : ;//On est dans le père
                token = 0;
                int child_pid = pid;
                printf("On est dans le père, pid = ");
                printf("%d\n",pid = getpid());
                //gettimeofday(tp,NULL);
                //td = tp->tv_usec;
                pipe_papa = open(FILE_PATH,O_RDONLY);
                if (-1 == pipe_papa)
                        printf("pb open pere\n");
                
                if (1 != read(pipe_papa,&token,1)){
                    printf("pb read pere\n");
                    exit(-1);
                    
                }
                printf("Thread papa: Token = %c\n",token);
                
                //gettimeofday(tp,NULL);
                //tf = tp->tv_usec;
                //float delta = ((float)tf-(float)td);
                
                //float d = NB_ECHANGES*64./delta;
                //printf("td = %lu, tf = %lu, delta = %f, Debit: %f Mb/s\n",td,tf,delta,d);
    }
    unlink(FILE_PATH);
    return 0;
}
