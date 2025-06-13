/*----------------------------------------------------------------------------*
 * fichier : fifo.h                                                           *
 * gestion d'une file (type FIFO) circulaire	                              *
 *----------------------------------------------------------------------------*/

#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdint.h>

/*----------------------------------------------------------------------------*
 * declaration des constantes                                                 *
 *----------------------------------------------------------------------------*/

/*
 * taille maximale de la FIFO
 */
#define TAILLE_FIFO 8

/*----------------------------------------------------------------------------*
 * declaration des structures                                                 *
 *----------------------------------------------------------------------------*/

/*
 * structure definissant une file
 */
typedef struct {
    // tableau qui stocke les donnees de la file
    uint16_t tab[TAILLE_FIFO];
    // taille de la file
    uint8_t fifo_taille;
    // index de tete de la file
    // pour rappel, la tete d'une file designe le premier element qui a ete
    // insere
    uint8_t fifo_tete;
    // index de queue de la file
    // pour rappel, la queue d'une file designe le dernier element qui a ete
    // insere
    uint8_t fifo_queue;
} FIFO;

/*----------------------------------------------------------------------------*
 * declaration des prototypes                                                 *
 *----------------------------------------------------------------------------*/

/*
 * initialise la file circulaire
 * entre  : pointeur sur la file a initialiser
 * sortie : sans
 * description : initialise la file circulaire
 */
void fifo_init(FIFO *f);

/*
 * ajoute un element a la file
 * entre  : pointeur sur la file ou il faut ajouter l'element
 *          element a ajouter
 * sortie : -1 si succes, O si erreur
 * description : ajoute un element a la file
 */
int fifo_ajoute(FIFO *f, uint16_t d);

/*
 * retire un element a la file
 * entre  : pointeur sur la file ou il faut retirer l'element
 *          pointeur utiliser pour recuperer la valeur retiree de la file
 * sortie : -1 si succes, O si erreur
 * description : retire un element de la file
 */
int fifo_retire(FIFO *f, uint16_t *d);

#endif