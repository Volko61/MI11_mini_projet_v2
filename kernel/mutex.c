/*----------------------------------------------------------------------------*
 * fichier : mutex.c                                                            *
 * gestion des mutex pour le mini-noyau temps reel                       *
 *----------------------------------------------------------------------------*/

#include "mutex.h"
#include "fifo.h"
#include "noyau_prio.h"
#include "noyau_file_prio.h"
#include <stdio.h>

#define NO_OWNER_TASK_ID 0xFFFF

/*----------------------------------------------------------------------------*
 * declaration des structures                                                 *
 *----------------------------------------------------------------------------*/

typedef struct {
    FIFO wait_queue;    // File d'attente des tâches en attente
    uint16_t owner_id;  // ID de la tâche propriétaire
    int8_t ref_count;   // Compteur de références
} MUTEX;

/*----------------------------------------------------------------------------*
 * variables globales internes                                                *
 *----------------------------------------------------------------------------*/

MUTEX _mutex[MAX_MUTEX];

/*----------------------------------------------------------------------------*
 * definition des fonctions                                                   *
 *----------------------------------------------------------------------------*/

void m_init(void) {
    register MUTEX *m = _mutex;
    register unsigned j;

    for (j = 0; j < MAX_MUTEX; j++) {
        fifo_init(&(m->wait_queue));
        m->owner_id = NO_OWNER_TASK_ID;
        m->ref_count = -1;
        m++;
    }
}

uint8_t m_create(void) {
    register unsigned n = 0;
    register MUTEX *m = &_mutex[n];

    _lock_();
    while (m->ref_count != -1 && n < MAX_MUTEX) {
        n++;
        m = &_mutex[n];
    }
    if (n < MAX_MUTEX) {
        fifo_init(&(m->wait_queue));
        m->ref_count = 0;
        m->owner_id = NO_OWNER_TASK_ID;
    } else {
        printf("Erreur : aucun mutex disponible\n");
        n = MAX_MUTEX;
    }
    _unlock_();
    return n;
}

void m_acquire(uint8_t n) {
    if (n >= MAX_MUTEX) {
        printf("Erreur : index de mutex invalide (%d)\n", n);
        noyau_exit();
    }

    register MUTEX *m = &_mutex[n]; // accède au mutex demandé dans le tableau des mutexs
    if (m->ref_count == -1) {
        printf("Erreur : mutex %d non créé\n", n);
        noyau_exit();
    }

    _lock_();
    uint16_t tc = noyau_get_tc();
    if (m->owner_id == NO_OWNER_TASK_ID) {
        m->owner_id = tc;
        m->ref_count = 1;
    } else if (m->owner_id == tc) {
        m->ref_count++;
    } else {
        uint16_t prio_tc = tc >> 3;
        uint16_t prio_owner = m->owner_id >> 3;
        if (prio_tc < prio_owner) {
        // if (prio_tc > prio_owner) { 
            // la tache en attentente du mutex a une priorité supérieur
            file_echange(tc, m->owner_id);
            printf("tache en attente du mutex : %d", tc);
            fifo_ajoute(&(m->wait_queue), tc); // met la tache demandant en attente
            //noyau_set_status(tc, SUSP);
            noyau_get_p_tcb(tc)->status = SUSP;  // pk pas 
            file_retire(m->owner_id); 
            schedule();
        } else {
            printf("tache en attente du mutex : %d", tc);
            fifo_ajoute(&(m->wait_queue), tc);
            dort();
        }
    }
    _unlock_();
}

void m_release(uint8_t n) {
    if (n >= MAX_MUTEX) {
        printf("Erreur : index de mutex invalide (%d)\n", n);
        noyau_exit();
    }

    register MUTEX *m = &_mutex[n];
    if (m->ref_count == -1) {
        printf("Erreur : mutex %d non créé\n", n);
        noyau_exit();
    }
    if (m->owner_id != noyau_get_tc()) {
        printf("Erreur : la tâche %d ne détient pas le mutex %d (propriétaire : %d)\n",
               noyau_get_tc(), n, m->owner_id);
        noyau_exit();
    }

    _lock_();
    m->ref_count--;
    if (m->ref_count == 0) {
        uint16_t tc = noyau_get_tc();
        uint16_t next_task = NO_OWNER_TASK_ID;
        if (m->wait_queue.fifo_taille > 0) {
            print("fifo attentente mutex taille %d", m->wait_queue.fifo_taille );
            if (fifo_retire(&(m->wait_queue), &next_task) == 0) {
                 printf("Valeur : %u\n", m->wait_queue);
                printf("je rentre dans la condition");
                printf("Erreur : échec de fifo_retire pour le mutex %d\n", n);
                _unlock_();
                noyau_exit();
            }
            uint16_t prio_tc = tc >> 3;
            uint16_t num_tc = tc & 7;

            uint16_t prio_next = next_task >> 3;
            uint16_t num_next = tc & 7;

            // if (prio_tc < prio_next) {
            if ((_id[prio_tc][num_tc] == next_task )&&(_id[prio_next][num_next] == tc)){

                file_echange(tc, next_task);
            }
            m->owner_id = next_task;
            m->ref_count = 1;
            printf("je vais planter là num tache : %u\n", next_task);
            reveille(next_task);
        } else {
            m->owner_id = NO_OWNER_TASK_ID;
        }
    }
    _unlock_();
}

void m_destroy(uint8_t n) {
    if (n >= MAX_MUTEX) {
        printf("Erreur : index de mutex invalide (%d)\n", n);
        noyau_exit();
    }

    register MUTEX *m = &_mutex[n];
    if (m->ref_count == -1) {
        printf("Erreur : mutex %d non créé\n", n);
        noyau_exit();
    }
    if (m->ref_count > 0) {
        printf("Erreur : le mutex %d est détenu par la tâche %d\n", n, m->owner_id);
        noyau_exit();
    }

    _lock_();
    fifo_init(&(m->wait_queue));
    m->ref_count = -1;
    m->owner_id = NO_OWNER_TASK_ID;
    _unlock_();
}