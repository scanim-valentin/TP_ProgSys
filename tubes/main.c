/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>
/* pour la gestion des signaux */
#include <signal.h>



void handler_parent(int sig){ //gestion des signaux par le père
    printf("Signal reçu par le parent\n");
    exit(0);
}


void handler_child(int sig){ //gestion des signaux par l'enfant 
    printf("--- Signal reçu par l'enfant\n");
    exit(0);
}

void handler_SIGCHLD(int sig){
    printf("L'enfant est mort\n");
}

int main(){
    struct sigaction action; /* structure de mise en place du handler */ 
    int pid = fork(); // Retourne 0 pour le fils, le PID du fils pour le père, -1 si erreur
    switch(pid){
            
            case(-1) :
                printf("Echec du fork\n");
                exit(1);
                break;
                
            case(0) : //On est dans le fils
                if(-1 == (pid = getpid()) ){
                    printf("--- Echec lors de l'appel de getpid()\n");
                    exit(1);
                }
                printf("--- On est dans le fils, pid = ");
                printf("%d\n",pid);
                
                //Mise en place du handler
                action.sa_handler = handler_child;
                if(-1 == sigaction(SIGUSR1, &action, NULL)){
                    printf("--- Echec lors de l'appel de sigaction()\n");
                }
                printf("--- Mise en attente du fils ...\n");
                while(1){
            
                }
                
                
                
            default : ;//On est dans le père
                
                int child_pid = pid;
                struct sigaction action_sigchld; /* structure de mise en place du handler */ 
                
                    if(-1 == (pid = getpid()) ){
                    printf("Echec lors de l'appel de getpid()\n");
                    exit(1);
                }
                printf("On est dans le père, pid = ");
                printf("%d\n",pid);

                    
                //Mise en place du handler
                action.sa_handler = handler_parent;
                if(-1 == sigaction(SIGUSR1, &action, NULL)){
                    printf("Echec lors de l'appel de sigaction()\n");
                }
                
                action_sigchld.sa_handler = handler_SIGCHLD;
                if(-1 == sigaction(SIGCHLD, &action_sigchld, NULL)){
                    printf("Echec lors de l'appel de sigaction()\n");
                }
                
                sleep(2);
                printf("Envoi de SIGUSR1 au fils\n");
                if(-1 == kill(child_pid,SIGUSR1)){
                    printf("Echec lors de l'appel de kill\n");
                    exit(1);
                }
                printf("Mise en attente du père ...\n");
                while(1){
                    
                }
                
                exit(0);
                break;
    }
}
