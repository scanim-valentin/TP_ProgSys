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
/* le temps */
#include <sys/time.h>

#define NB_ECHANGES 1000

int main(){
    struct timeval* restrict tp = (struct timeval*)malloc(sizeof(struct timeval));
    gettimeofday(tp,NULL);
    printf("Le temps : %lu\n",tp->tv_usec );
    unsigned long tf, td;
    int p_pere[2];
    int p_fils[2];
	char buffer = ' '; //Création du buffer
	char token = 'a';
    if(pipe(p_pere)){ //Création du pipe
    	printf("Echec lors de la creation du pipe du pere ( pipe(p) )");
    }
    if(pipe(p_fils)){ //Création du pipe
    	printf("Echec lors de la creation du pipe du fils ( pipe(p) )");
    }
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
                for(int k = 0; k<NB_ECHANGES;k++){
                    if (write(p_fils[1],&token,1) != 1)
                        printf("pb write fils\n");
                    //printf("Fiston: Attente read\n");
                    if (read(p_pere[0],&token,1) != 1)
                        printf("pb read pere\n");
                    //printf("Fiston : Token = \"%d\" \n",(int)token);
                    token++;                    
                }
                break;

            default : ;//On est dans le père
                int child_pid = pid;
                printf("On est dans le père, pid = ");
                printf("%d\n",pid = getpid());
                gettimeofday(tp,NULL);
                td = tp->tv_usec;
                for(int k = 0; k<NB_ECHANGES;k++){
                    if (read(p_fils[0],&buffer,1) != 1)
                        printf("pb read fils\n");
                    //printf("Papa : Token = \"%d\" \n",(int)buffer);
                    buffer++;
                    if (write(p_pere[1],&buffer,1) != 1)
                        printf("pb write pere\n");
                }
                gettimeofday(tp,NULL);
                tf = tp->tv_usec;
                float delta = ((float)tf-(float)td);
                
                float d = NB_ECHANGES*64./delta;
                printf("td = %lu, tf = %lu, delta = %f, Debit: %f Mb/s\n",td,tf,delta,d);
    }

    return 0;
}
