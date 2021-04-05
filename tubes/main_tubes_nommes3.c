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

#define PAPA_PATH "/tmp/pipe_papa"
#define FILS_PATH "/tmp/pipe_fils"
#define NB_ECHANGES 1000

int main(){
    //Calcul du debit
    struct timeval* restrict tp = (struct timeval*)malloc(sizeof(struct timeval));
    gettimeofday(tp,NULL);
    printf("Le temps : %lu\n",tp->tv_usec );
    unsigned long tf, td;
    
    unlink(FILS_PATH);unlink(PAPA_PATH);
	char token = 0;
    if(-1 == mkfifo(FILS_PATH,S_IRUSR | S_IWUSR | S_IRGRP)){ //Création du pipe
    	printf("Echec lors de la creation du pipe du fils ( mkfifo )\n");
        exit(-1);
    }
    if(-1 == mkfifo(PAPA_PATH,S_IRUSR | S_IWUSR | S_IRGRP)){ //Création du pipe
    	printf("Echec lors de la creation du pipe du fils ( mkfifo )\n");
        exit(-1);
    }
    int pipe_papa,pipe_fils;
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
                
                if (-1 == (pipe_papa = open(PAPA_PATH, O_WRONLY)) ){
                        printf("pb open pipe_papa fils\n");
                        exit(-1);
                }

                if (-1 == (pipe_fils = open(FILS_PATH, O_RDONLY)) ){
                    printf("pb open pipe_fils fils\n");
                    exit(-1);
                }
                for(int k = 0;k < NB_ECHANGES;k++){
                     //                   sleep(1);
                    if (1 != write(pipe_papa,&token,1)){
                        printf("pb write fils\n");
                        exit(-1);
                    }
                    //sleep(1);
                    if (1 != read(pipe_fils,&token,1)){
                        printf("pb read fils\n");
                        exit(-1);
                    }
                    //printf("Proc. fiston: Token = %d\n",(int)token);
                    
                    token++;
                }
                break;

            default : ;//On est dans le père
                token = 0;
                int child_pid = pid;
                printf("On est dans le père, pid = ");
                printf("%d\n",pid = getpid());

                if (-1 == (pipe_papa = open(PAPA_PATH, O_RDONLY)) ){
                        printf("pb open pipe_papa pere\n");
                        exit(-1);
                }
                gettimeofday(tp,NULL);
                if (-1 == (pipe_fils = open(FILS_PATH, O_WRONLY)) ){
                    printf("pb open pipe_fils pere\n");
                    exit(-1);
                }
                for(int k = 0;k < NB_ECHANGES;k++){
                    //sleep(1);
                    if (1 != read(pipe_papa,&token,1)){
                        printf("pb read pere\n");
                        exit(-1);                    
                    }
                    //printf("Proc. papa: Token = %d\n",(int)token);
                    token++;
                                    //sleep(1);
                    if (1 != write(pipe_fils,&token,1)){
                        printf("pb write pere\n");
                        exit(-1);
                    }
                }
                td = tp->tv_usec;
                gettimeofday(tp,NULL);
                tf = tp->tv_usec;
                float delta = ((float)tf-(float)td);
                
                float d = NB_ECHANGES*64./delta;
                printf("td = %lu, tf = %lu, delta = %f, Debit: %f Mb/s\n",td,tf,delta,d);
                printf("Mission termine\n");
    }
    unlink(FILS_PATH);
    unlink(PAPA_PATH);
    return 0;
}
