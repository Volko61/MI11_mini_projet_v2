/*----------------------------------------------------------------------------*
 * fichier : mutex.c                                                            *
 * gestion des mutex pour le mini-noyau temps reel                       *
 *----------------------------------------------------------------------------*/

#include "mutex.h"
#include "fifo.h"
#include "noyau_prio.h"
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

    register MUTEX *m = &_mutex[n];
    if (m->ref_count == -1) {
        printf("Erreur : mutex %d non créé\n", n);
        noyau_exit();
    }

    _lock_();
    uint16_t tc = noyau_get_tc();
    if (m->owner_id == NO_OWNER_TASK_ID) {
        // Mutex libre, on l'acquiert
        m->owner_id = tc;
        m->ref_count = 1;
    } else if (m->owner_id == tc) {
        // Réentrance : même tâche qui veut acquérir le mutex
        m->ref_count++;
    } else {
        // Mutex détenu par une autre tâche
        uint16_t prio_tc = tc >> 3;
        uint16_t prio_owner = m->owner_id >> 3;
        
        // Ajouter la tâche courante à la file d'attente
        fifo_ajoute(&(m->wait_queue), tc);
        
        if (prio_tc < prio_owner) {
            // Inversion de priorité : la tâche courante a une priorité plus haute
            // On échange les priorités (héritage de priorité)
            file_echange(tc, m->owner_id);
            
            // Suspendre la tâche courante
            noyau_get_p_tcb(tc)->status = SUSP; 
            
            // Retirer le propriétaire de la file prête et le remettre avec nouvelle priorité
            file_retire(m->owner_id);
            file_ajoute(m->owner_id); // Ajout avec priorité héritée
            
            // Déclencher l'ordonnancement
            schedule();
        } else {
            // Pas d'inversion de priorité, on endort simplement la tâche
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
        // Plus de références, on libère le mutex
        uint16_t tc = noyau_get_tc();
        uint16_t next_task = NO_OWNER_TASK_ID;
        
        if (m->wait_queue.fifo_taille > 0) {
            // Il y a des tâches en attente
            if (fifo_retire(&(m->wait_queue), &next_task) != 0) {
                printf("Erreur : échec de fifo_retire pour le mutex %d\n", n);
                _unlock_();
                noyau_exit();
            }
            
            // Restaurer la priorité originale de la tâche courante si nécessaire
            uint16_t prio_tc = tc >> 3;
            uint16_t prio_next = next_task >> 3;
            
            // Donner le mutex à la prochaine tâche
            m->owner_id = next_task;
            m->ref_count = 1;
            
            // Réveiller la tâche qui obtient le mutex
            reveille(next_task);
            
            // Si la tâche qui obtient le mutex a une priorité plus haute,
            // déclencher l'ordonnancement
            if (prio_next < prio_tc) {
                // Remettre la tâche courante dans la file avec sa priorité originale
                file_retire(tc);
                file_ajoute(tc);
                schedule();
            }
        } else {
            // Aucune tâche en attente, le mutex devient libre
            m->owner_id = NO_OWNER_TASK_ID;
            
            // Restaurer la priorité originale si elle avait été modifiée
            // (cette partie dépend de l'implémentation exacte de l'héritage de priorité)
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
    // Vérifier qu'il n'y a pas de tâches en attente
    if (m->wait_queue.fifo_taille > 0) {
        printf("Erreur : le mutex %d a encore des tâches en attente\n", n);
        _unlock_();
        noyau_exit();
    }
    
    // Réinitialiser le mutex
    fifo_init(&(m->wait_queue));
    m->ref_count = -1;
    m->owner_id = NO_OWNER_TASK_ID;
    _unlock_();
}