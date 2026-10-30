#ifndef COLIS_H
#define COLIS_H

#include <stdint.h>

typedef struct {
    uint32_t identifiant;
    char destination;
    float poids;
} colis_t;

int colis_init(colis_t *c, uint32_t id, char dest, float poids);




#endif