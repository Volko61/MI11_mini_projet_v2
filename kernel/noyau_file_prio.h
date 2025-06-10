/*----------------------------------------------------------------------------*
 * fichier : noyau_file_prio.h                                                *
 * gestion de la file d'attente des taches pretes et actives                  *
 * ce fichier declare toutes les primitives de base                           *
 *----------------------------------------------------------------------------*/

#ifndef __NOYAU_FILE_H__
#define __NOYAU_FILE_H__

#include <stdint.h>

/*----------------------------------------------------------------------------*
 * declaration des constantes                                                 *
 *----------------------------------------------------------------------------*/

/*
 * nombre maximum de taches dans le systeme
 */
#define MAX_TACHES_FILE  8
#define MAX_PRIO         8

#define MAX_TACHES_NOYAU MAX_TACHES_FILE * MAX_PRIO

/*
 * numero de tache impossible, utilise pour savoir si la file est initialisee
 * ou non
 */
#define F_VIDE      MAX_TACHES_FILE

/*----------------------------------------------------------------------------*
 * prototypes des fonctions de gestion de la file                             *
 * voir le fichier noyau_file.c pour avoir le comportement des fonctions      *
 *----------------------------------------------------------------------------*/

void file_init(void);
void file_ajoute(uint16_t n, uint16_t id_tache);
void file_retire(uint16_t t);
uint16_t file_suivant(void);
void file_affiche_queue(void);
void file_affiche(void);


#endif //__NOYAU_FILE_H__
