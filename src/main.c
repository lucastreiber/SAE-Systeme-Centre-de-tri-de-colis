#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#include "colis.h"
#include "zone.h"

int main(){

    pid_t pid_tri = fork();
    if(pid_tri < 0){perror("fork tri"); return EXIT_FAILURE;}

    //processus fils tri
    if(pid_tri == 0){


        exit(EXIT_SUCCESS);
    }
    //processus père main
    pid_t pid_affichage = fork();
    if(pid_affichage < 0){perror("fork affichage"); return EXIT_FAILURE;}

    //processus fils affichage
    if(pid_affichage == 0){



        exit(EXIT_SUCCESS);
    }

    //processus père
    int terminer = 0;
    char commande[32];   

    while(!terminer){
        printf("Tape une commande (COLIS, ETAT, QUIT) :");
        if(scanf("%s",commande) != 1){break;}   //Cas particulier EOF (j'ai pas tout compris mais en gros c'est quand tu fait CTRL D (linux) ou Z (Windows) quand tu écris et du coup ça lui envoie un signiale qui lui dit il y a plus rien à lire donc il change plus jamais la varaible donc il boulce pour tjrs)

        if(strcmp(commande, "QUIT") == 0){
            terminer = 1;
        }
        else if(strcmp(commande, "ETAT") == 0){
            //lancer la fonction pour afficher l'etat

        }
        else if(strcmp(commande, "COLIS") == 0){
            unsigned int id;
            char dest;
            float poids;

            if (scanf("%u %c %f", &id, &dest, &poids) == 3) {
                colis_t colis;
                if(colis_init(&colis, id, dest, poids) == 0){
                    //envoie le colis
                }
            }else{
                printf("Erreur de syntaxe :\n\tCOLIS <id> <destination> <poids>\nl'id est un nombre, la destination est un caractère parmis A, B et C et le poids est un nombre à virgule");
            }
        }
    }


    //On attends que tous les fils se terminent
    for(size_t i; i < 2; i++){
        wait(NULL);
    }

    return EXIT_SUCCESS;
}