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
/* pour la gestion des tubes */
#include <unistd.h>




int main(){
  
    int p[2];
	char buffer[100]; //Création du buffer
	strcpy(buffer,"");
	
	////////////////////////////////////////////////////////////////////
	int pid = fork(); // Retourne 0 pour le fils, le PID du fils pour le père, -1 si erreur
    if(pipe(p)){ //Création du pipe
    	printf("Echec lors de la creation du pipe ( pipe(p) )");
    }
    
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
                if (write(p[1],"La somme des forces est égale à la masse par l'accélération",100) != 100){
     				printf("pb ecriture\n");
      				exit(-1);
   				}
   				exit(0);
                            
                
                
            default : ;//On est dans le père
                
                int child_pid = pid;
                printf("On est dans le père, pid = ");
                printf("%d\n",pid);
				if (read(p[0],buffer,100)!=21){
      				printf("pb lecture\n");
      				exit(-1);
      			}                
               
                exit(0);

    }
}
