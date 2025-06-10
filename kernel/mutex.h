/*
 * mutex.h
 *
 *  Created on: 15 mai 2025
 *      Author: bonnetst
 */

#ifndef KERNEL_MUTEX_H_
#define KERNEL_MUTEX_H_

#include <stdint.h>


#define MAX_MUTEX 16

/* m_init
 *
 * initialise le tableau des mutex de telle manière qu'ils soient tous disponibles
 *
 */
void m_init(void);

/* m_create
 *
 * Crée un mutex. Retourne le numéro de mutex si ok, MAX_MUTEX sinon.
 *
 */
uint8_t m_create(void);

/* m_acquire
 *
 * Acquiert le mutex n. Les mutex sont ré-entrants : si une tâche ré-acquiert un mutex dont elle est
 * déjà propriétaire, ce n'est pas une erreur, elle ne dormira pas dessus.
 *
 */
void m_acquire(uint8_t n);
void m_release(uint8_t n);
void m_destroy(uint8_t n);


#endif /* KERNEL_MUTEX_H_ */
