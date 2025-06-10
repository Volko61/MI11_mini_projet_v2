/*----------------------------------------------------------------------------*
 * fichier : mutex.c
 * gestion des mutex avec héritage de priorité par échange d'identités.
 *----------------------------------------------------------------------------*/

#include "mutex.h"
#include "fifo.h"
#include "noyau_prio.h" // Contient les déclarations du noyau
#include "noyau_file_prio.h" // IMPORTANT : Pour file_echange()

#include <stdio.h>

#define NO_OWNER_TASK_ID 0xFFFF // Utilisation d'une valeur improbable pour un ID de tâche

/*----------------------------------------------------------------------------*
 * declaration des structures
 *----------------------------------------------------------------------------*/
typedef struct {
    FIFO wait_queue;
    uint16_t owner_id; // Utilise uint16_t pour correspondre aux ID de tâches
    int8_t ref_count;
} MUTEX;

/*----------------------------------------------------------------------------*
 * variables globales internes
 *----------------------------------------------------------------------------*/
static MUTEX _mutex[MAX_MUTEX];

/*----------------------------------------------------------------------------*
 * Fonctions (seules m_acquire et m_release sont significativement modifiées)
 *----------------------------------------------------------------------------*/

void m_init(void) {
    register MUTEX *m = _mutex;
    register unsigned j;
    for (j = 0; j < MAX_MUTEX; j++, m++) {
        m->owner_id = NO_OWNER_TASK_ID;
        m->ref_count = -1;
    }
}

uint8_t m_create() {
    register unsigned n = 0;
    register MUTEX *m = &_mutex[n];
    _lock_();
    while(m->ref_count != -1 && n < MAX_MUTEX) {
        n++;
        m = &_mutex[n];
    }
    if (n < MAX_MUTEX) {
        fifo_init(&(m->wait_queue)); 
        m->ref_count = 0;
        m->owner_id = NO_OWNER_TASK_ID;
    } else {
        n = MAX_MUTEX;
    }
    _unlock_();
    return n;
}

/*
 * MODIFIÉ : Acquiert le mutex n avec héritage de priorité.
 */
void m_acquire(uint8_t n) {
    if (n >= MAX_MUTEX) { /* ... gestion erreur ... */ return; }

    register MUTEX *m = &_mutex[n];
    if (m->ref_count == -1) { /* ... gestion erreur ... */ return; }

    _lock_();
    
    uint16_t current_task_id = noyau_get_tc();

    // Si le mutex est déjà pris
    if (m->ref_count > 0) {
        // Cas ré-entrant : la tâche est déjà propriétaire
        if (m->owner_id == current_task_id) {
            m->ref_count++;
        } else {
            // Cas de conflit : une autre tâche est propriétaire
            uint16_t owner_task_id = m->owner_id;
            
            // *** DÉBUT DE LA LOGIQUE D'HÉRITAGE DE PRIORITÉ ***
            // On suppose l'existence de noyau_get_prio()
            if (noyau_get_prio(current_task_id) < noyau_get_prio(owner_task_id)) {
                // La tâche courante est plus prioritaire : on échange les identités.
                file_echange(current_task_id, owner_task_id);
            }
            // *** FIN DE LA LOGIQUE D'HÉRITAGE DE PRIORITÉ ***

            // Dans tous les cas de conflit, la tâche courante doit attendre.
            fifo_ajoute(&(m->wait_queue), current_task_id);
            dort();
        }
    } else {
        // Le mutex est libre, on le prend.
        m->owner_id = current_task_id;
        m->ref_count = 1;
    }
    _unlock_();
}

/*
 * MODIFIÉ : Libère le mutex n avec restauration de priorité.
 */
void m_release(uint8_t n) {
    if (n >= MAX_MUTEX) { /* ... gestion erreur ... */ return; }

    register MUTEX *m = &_mutex[n];
    if (m->ref_count <= 0 || m->owner_id != noyau_get_tc()) { /* ... gestion erreur ... */ return; }

    _lock_();
    
    m->ref_count--;
    
    // Si la tâche a complètement libéré le mutex
    if (m->ref_count == 0) {
        // S'il y a des tâches en attente
        if (m->wait_queue.fifo_taille > 0) {
            uint16_t new_task_id;
            uint16_t current_task_id = noyau_get_tc();
            
            // On récupère la prochaine tâche en attente
            fifo_retire(&(m->wait_queue), &new_task_id);

            // *** DÉBUT DE LA RESTAURATION DE PRIORITÉ ***
            // On restaure les identités pour annuler l'héritage.
            file_echange(current_task_id, new_task_id);
            // *** FIN DE LA RESTAURATION DE PRIORITÉ ***
            
            // On assigne le mutex à la nouvelle tâche
            m->owner_id = new_task_id;
            m->ref_count = 1;
            
            // On la réveille. Elle a maintenant sa priorité d'origine (haute).
            reveille(new_task_id);
        } else {
            // Personne n'attend, le mutex redevient libre.
            m->owner_id = NO_OWNER_TASK_ID;
        }
    }
    _unlock_();
}

void m_destroy(uint8_t n) {
    if (n >= MAX_MUTEX) { /* ... gestion erreur ... */ return; }
    register MUTEX *m = &_mutex[n];
    if (m->ref_count > 0) { /* ... gestion erreur ... */ return; }

    _lock_();
    m->ref_count = -1;
    m->owner_id = NO_OWNER_TASK_ID;
    _unlock_();
}