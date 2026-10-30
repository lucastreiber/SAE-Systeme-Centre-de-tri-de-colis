#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "colis.h"

int colis_init(colis_t *c, uint32_t id, char dest, float pds){
    // Vérification du pointeur c
    if (c == NULL) {
        errno = EINVAL;
        perror("colis_init : pointeur NULL");
        return EXIT_FAILURE;
    }
    // Vérification de la destination
    if (dest != 'A' && dest != 'B' && dest != 'C') {
        errno = EINVAL;
        perror("colis_init : destination incorrecte");
        return EXIT_FAILURE;
    }
    // Vérification du poids
    if (pds <= 0) {
        errno = EINVAL;
        perror("colis_init : poids incorrect");
        return EXIT_FAILURE;
    }

    c->identifiant = id;
    c->destination = dest;
    c->poids = pds;
    return EXIT_SUCCESS;
}