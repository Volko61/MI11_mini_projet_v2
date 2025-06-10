/*----------------------------------------------------------------------------*
 * fichier : sem.c                                                            *
 * gestion des semaphores pour le mini-noyau temps reel                       *
 *----------------------------------------------------------------------------*/

#include "sem.h"

#include "fifo.h"
#include "noyau_prio.h"
#include <stdio.h>
/*----------------------------------------------------------------------------*
 * declaration des structures                                                 *
 *----------------------------------------------------------------------------*/

/*
 * structure definissant un semaphore
 */
typedef struct {
    FIFO file;
    int8_t valeur;
} SEMAPHORE;

/*----------------------------------------------------------------------------*
 * variables globales internes                                                *
 *----------------------------------------------------------------------------*/

/*
 * variable stockant tous les semaphores du systeme
 */
SEMAPHORE _sem[MAX_SEM];

/*----------------------------------------------------------------------------*
 * definition des fonctions                                                   *
 *----------------------------------------------------------------------------*/

/*
 * /!!!!\ NOTE IMPORTANTE /!!!!\
 * pour faire les verifications de file, on pourra utiliser la variable de
 * file fifo_taille et la mettre a -1 dans le cas ou la file n'est pas
 * utilisee
 */

/*
 * initialise les sempaphore du systeme
 * entre  : sans
 * sortie : sans
 * description : initialise l'ensemble des semaphores du systeme
 */
void s_init(void) {
	register SEMAPHORE *s = _sem;
	register unsigned j;
		   
   
	for (j = 0; j < MAX_SEM; j++)
	{
		s->file.fifo_taille = -1;
		s++;
	}
}

/*
 * cree un semaphore
 * entre  : valeur du semaphore a creer
 * sortie : numero du semaphore cree
 * description : cree un semaphore
 *               en cas d'erreur, le noyau doit etre arrete
 */
uint8_t s_cree(int8_t valeur) {
	register SEMAPHORE *s = _sem;
	register unsigned n = 0;

	_lock_();
	/* Rechercher un sem libre */
	while(s->file.fifo_taille != -1 && n < MAX_SEM)
	{
		n++;
		s++;
	}

	if (n < MAX_SEM)
	{
		fifo_init(&(s->file));
		s->valeur = valeur;
	}
	else
	{
		printf("Tnetative de creer unn sémaphore qui a échouée");
		noyau_exit();
	}
	_unlock_();

	return(n);	   
}

/*
 * ferme un semaphore pour qu'il puisse etre reutilise
 * entre  : numero du semaphore a fermer
 * sortie : sans
 * description : ferme un semaphore
 *               en cas d'erreur, le noyau doit etre arrete
 */
void s_close(uint8_t n) {
	register SEMAPHORE *s = &_sem[n];

	_lock_();

	/* V‚rifier sem cr‚e */
	if (s->file.fifo_taille == -1)
	{
		printf("Ce sémaphore n'a pas déjà été créer");
		noyau_exit();
	}

	s->file.fifo_taille = -1;
	_unlock_();		   
}

/*
 * tente de prendre le semaphore dont le numero est passe en parametre
 * entre  : numero du semaphore a prendre
 * sortie : sans
 * description : prend le semaphore
 *               si echec, la tache doit etre suspendue
 *               en cas d'erreur, le noyau doit etre arrete
 */
void s_wait(uint8_t n) {

	register SEMAPHORE *s = &_sem[n];

	_lock_();

	if (s->file.fifo_taille == -1)
	{
		printf("Ce sémaphore n'a pas déjà été créer");
		noyau_exit();
	}

	s->valeur--;
	if (s->valeur < 0)
	{
		fifo_ajoute(&s->file, noyau_get_tc());
		dort();
	}

	_unlock_();		
}

/*
 * libere un semaphore
 * entre  : numero du semaphore a liberer
 * sortie : sans
 * description : libere un semaphore
 *               si des taches sont en attentes, la premiere doit etre reveillee
 *               en cas d'erreur, le noyau doit etre arrete
 */
void s_signal(uint8_t n) {
	register SEMAPHORE *s = &_sem[n];
	uint8_t t;

	_lock_();

	if (s->file.fifo_taille == -1)
	{
		printf("Ce sémaphore n'a pas déjà été créer");
		noyau_exit();
	}

	s->valeur++;

	if (s->valeur <= 0)
	{
		fifo_retire(&s->file, (uint8_t *) &t);
		reveille(t);
	}
	_unlock_();	
}





