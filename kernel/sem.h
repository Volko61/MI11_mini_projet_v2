/*----------------------------------------------------------------------------*
 * fichier : sem.h                                                            *
 * gestion des semaphores pour le mini-noyau temps reel                       *
 *----------------------------------------------------------------------------*/

#ifndef __SEM_H__
#define __SEM_H__

#include <stdint.h>

#include "fifo.h"

/*----------------------------------------------------------------------------*
 * declaration des constantes                                                 *
 *----------------------------------------------------------------------------*/

#define MAX_SEM        16

/*----------------------------------------------------------------------------*
 * declaration des prototypes                                                 *
 *----------------------------------------------------------------------------*/

void s_init(void);
uint8_t s_cree(int8_t valeur);
void s_close(uint8_t n);
void s_wait(uint8_t n);
void s_signal(uint8_t n);

#endif

