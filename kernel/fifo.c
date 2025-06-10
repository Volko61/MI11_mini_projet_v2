/*----------------------------------------------------------------------------*
 * fichier : fifo.c                                                           *
 * gestion d'une file (type FIFO) circulaire	                              *
 *----------------------------------------------------------------------------*/

#include "fifo.h"

/*----------------------------------------------------------------------------*
 * definition des fonctions                                                   *
 *----------------------------------------------------------------------------*/

void fifo_init(FIFO *f)
{
	f->fifo_taille = f->fifo_queue = f->fifo_tete = 0;
}

int fifo_ajoute(FIFO *f, uint8_t d)
{
	if (f->fifo_taille >= TAILLE_FIFO)
	{
		return(0);
	}

	f->tab[f->fifo_queue++] = d;
	f->fifo_queue %= TAILLE_FIFO;
	f->fifo_taille++;
	return(-1);
}

int fifo_retire(FIFO *f, uint8_t *d)
{
	if (f->fifo_taille == 0)
	{
		return(0);
	}

	*d = f->tab[f->fifo_tete++];
	f->fifo_tete %= TAILLE_FIFO;
  	f->fifo_taille--;
	return(-1);
}
