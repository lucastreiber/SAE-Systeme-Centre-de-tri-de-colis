#ifndef COMMUN_H
#define COMMUN_H

#include <stdint.h>
#include <signal.h>
#include <sys/types.h>

#include "colis.h"
#include "zone.h"

#define OBJECTIF 20
#define NB_ZONES 3

#define FIFO_COMMANDES "/tmp/centre_tri_commandes"
#define FIFO_ETAT "/tmp/centre_tri_etat"
#define FICHIER_JOURNAL "journal.log"

#define SIG_OBJECTIF SIGUSR1
#define SIG_SYNCHRO SIGUSR2
#define SIG_ARRET SIGTERM

typedef enum {
    CMD_COLIS,
    CMD_ETAT,
    CMD_QUIT
} type_commande_t;

typedef struct {
    type_commande_t type;
    colis_t colis;
} commande_t;

typedef struct {
    zone_t zones[NB_ZONES];
    uint16_t nb_colis_total;
    float poids_total;
    colis_t dernier_colis;
} etat_centre_t;

void processus_tri(pid_t pid_affichage);
void processus_affichage(void);

void *thread_routage(void *arg);
void *thread_zone(void *arg);
void *thread_supervision(void *arg);

#endif
