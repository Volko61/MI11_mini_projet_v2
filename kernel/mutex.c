/*----------------------------------------------------------------------------*
 * fichier : mutex.c                                                            *
 * gestion des mutex pour le mini-noyau temps reel                       *
 *----------------------------------------------------------------------------*/

#include "mutex.h"
#include "fifo.h"
#include "noyau_prio.h" // Doit contenir les déclarations pour les fonctions noyau_
#include <stdio.h>

#define NO_OWNER_TASK_ID 0xFFFF

/*----------------------------------------------------------------------------*
 * declaration des structures                                                 *
 *----------------------------------------------------------------------------*/

typedef struct {
    FIFO wait_queue;    // File d'attente des tâches en attente
    uint16_t owner_id;  // ID de la tâche propriétaire
    int8_t ref_count;   // Compteur de références (pour mutex récursif)
    // Pour l'héritage de priorité, il serait utile de stocker la priorité d'origine
    // du propriétaire si elle a été augmentée, ou de gérer cela dans le TCB.
    // priority_t owner_original_priority; // Exemple
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
        m->ref_count = -1; // Marque le mutex comme non créé/disponible
        m++;
    }
}

uint8_t m_create(void) {
    register unsigned n = 0;
    register MUTEX *m = &_mutex[n]; // Pointeur vers le premier mutex

    _lock_(); // Début de la section critique
    while (n < MAX_MUTEX && m->ref_count != -1) { // Cherche un mutex non utilisé
        n++;
        if (n < MAX_MUTEX) { // Évite de déborder si tous les mutex sont utilisés
             m = &_mutex[n];
        }
    }

    if (n < MAX_MUTEX) {
        // fifo_init(&(m->wait_queue)); // Déjà fait dans m_init, mais peut être réaffirmé
        m->ref_count = 0;          // Créé, mais pas encore détenu
        m->owner_id = NO_OWNER_TASK_ID;
    } else {
        printf("Erreur : aucun mutex disponible\n");
        n = MAX_MUTEX; // Valeur d'erreur indiquant qu'aucun mutex n'a été créé
    }
    _unlock_(); // Fin de la section critique
    return (uint8_t)n; // n sera MAX_MUTEX en cas d'erreur
}

void m_acquire(uint8_t n) {
    if (n >= MAX_MUTEX) {
        printf("Erreur : index de mutex invalide (%d) lors de acquire\n", n);
        noyau_exit(); // Ou une autre gestion d'erreur
    }

    register MUTEX *m = &_mutex[n];
    if (m->ref_count == -1) {
        printf("Erreur : mutex %d non créé lors de acquire\n", n);
        noyau_exit();
    }

    _lock_();
    uint16_t current_task_id = noyau_get_tc();

    if (m->owner_id == NO_OWNER_TASK_ID) { // Mutex libre
        m->owner_id = current_task_id;
        m->ref_count = 1;
        // Si on stocke la priorité d'origine pour PIP :
        // m->owner_original_priority = noyau_get_task_priority(current_task_id);
    } else if (m->owner_id == current_task_id) { // Acquisition récursive par le propriétaire
        m->ref_count++;
    } else { // Mutex détenu par une autre tâche
        // Logique d'héritage de priorité (PIP)
        // On suppose que noyau_get_task_priority retourne la priorité effective
        // et que plus la valeur est petite, plus la priorité est haute.
        priority_t current_task_prio = noyau_get_task_priority(current_task_id);
        priority_t owner_task_prio = noyau_get_task_priority(m->owner_id);

        if (current_task_prio < owner_task_prio) {
            // La tâche courante est plus prioritaire que le propriétaire du mutex.
            // Il faut élever la priorité du propriétaire.
            noyau_promote_task_priority(m->owner_id, current_task_prio);
            // La fonction noyau_promote_task_priority devrait sauvegarder l'ancienne
            // priorité du m->owner_id et le replacer correctement dans la file des tâches prêtes si besoin.
        }

        // La tâche courante doit attendre
        fifo_ajoute(&(m->wait_queue), current_task_id);
        // noyau_set_status(current_task_id, SUSP); // Alternative
        noyau_get_p_tcb(current_task_id)->status = SUSP;
        schedule(); // Le scheduler choisira la tâche la plus prioritaire (potentiellement le propriétaire avec sa priorité élevée)
                    // Si dort() fait SUSP + schedule, on peut utiliser dort() ici.
                    // dort();
    }
    _unlock_();
}

void m_release(uint8_t n) {
    if (n >= MAX_MUTEX) {
        printf("Erreur : index de mutex invalide (%d) lors de release\n", n);
        noyau_exit();
    }

    register MUTEX *m = &_mutex[n];
    if (m->ref_count == -1) {
        printf("Erreur : mutex %d non créé lors de release\n", n);
        noyau_exit();
    }

    uint16_t current_task_id = noyau_get_tc();
    if (m->owner_id != current_task_id) {
        printf("Erreur : la tâche %d tente de relâcher le mutex %d non détenu (propriétaire : %d)\n",
               current_task_id, n, m->owner_id);
        noyau_exit();
    }

    _lock_();
    m->ref_count--;

    if (m->ref_count == 0) { // Dernière libération (pour les mutex récursifs)
        // Avant de potentiellement céder le mutex, restaurer la priorité de la tâche courante
        // si elle avait été modifiée à cause de l'héritage de priorité lié à CE mutex.
        // Cela suppose que noyau_restore_task_priority sait quelle était la priorité d'origine
        // ou la recalcule en fonction des autres mutex que la tâche pourrait détenir.
        noyau_restore_task_priority(current_task_id); // Doit être implémenté dans le noyau

        if (m->wait_queue.fifo_taille > 0) { // Y a-t-il des tâches en attente ?
            uint16_t next_owner_id;
            if (fifo_retire(&(m->wait_queue), &next_owner_id) != 0) { // 0 pour succès
                printf("Erreur : échec de fifo_retire pour le mutex %d\n", n);
                _unlock_(); // Important avant noyau_exit
                noyau_exit();
            }
            m->owner_id = next_owner_id;
            m->ref_count = 1; // Le nouveau propriétaire l'a acquis une fois
            // Si on stocke la priorité d'origine pour PIP dans le mutex :
            // m->owner_original_priority = noyau_get_task_priority(next_owner_id);

            // La tâche next_owner_id pourrait elle-même avoir besoin d'un boost de priorité
            // si une tâche de plus haute priorité attend un autre mutex qu'elle détient.
            // Pour l'instant, on la réveille simplement. Le mécanisme PIP s'appliquera
            // si next_owner_id essaie d'acquérir un autre mutex ou si une autre tâche
            // essaie d'acquérir un mutex détenu par next_owner_id.
            reveille(next_owner_id); // Rend la tâche prête et appelle schedule si nécessaire
        } else { // Personne n'attend
            m->owner_id = NO_OWNER_TASK_ID;
        }
        // Un schedule() peut être nécessaire ici si reveille() ne le fait pas
        // ou si la restauration de priorité a changé l'ordonnancement.
        // schedule(); // A évaluer en fonction de l'implémentation de reveille et noyau_restore_task_priority
    }
    _unlock_();
}

void m_destroy(uint8_t n) {
    if (n >= MAX_MUTEX) {
        printf("Erreur : index de mutex invalide (%d) lors de destroy\n", n);
        noyau_exit();
    }

    register MUTEX *m = &_mutex[n];
    _lock_(); // Section critique pour vérifier et modifier le mutex

    if (m->ref_count == -1) {
        _unlock_();
        printf("Erreur : mutex %d non créé lors de destroy\n", n);
        noyau_exit();
    }
    if (m->ref_count > 0) { // Ou m->owner_id != NO_OWNER_TASK_ID
        _unlock_();
        printf("Erreur : le mutex %d est détenu par la tâche %d et ne peut être détruit\n", n, m->owner_id);
        noyau_exit();
    }
    if (m->wait_queue.fifo_taille > 0) {
        _unlock_();
        // C'est un problème : des tâches attendent un mutex qui va être détruit.
        // Idéalement, il faudrait les réveiller avec un code d'erreur.
        printf("Erreur : des tâches attendent le mutex %d qui va être détruit. Destruction annulée.\n", n);
        noyau_exit(); // Ou retourner un code d'erreur
    }

    // Réinitialiser le mutex pour qu'il puisse être recréé
    fifo_init(&(m->wait_queue)); // Vider la file d'attente (devrait déjà être vide)
    m->ref_count = -1;
    m->owner_id = NO_OWNER_TASK_ID;
    _unlock_();
}