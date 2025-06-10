/*----------------------------------------------------------------------------*
 * fichier : noyau_file_prio.c                                                *
 * gestion de la file d'attente des taches pretes et actives                  *
 * la file est rangee dans un tableau. ce fichier definit toutes              *
 * les primitives de base                                                     *
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include "noyau_file_prio.h"
// recuperation du bon fichier selon l'architecture pour la fonction printf
#include "../io/serialio.h"


/*----------------------------------------------------------------------------*
 * variables communes a toutes les procedures                                 *
 *----------------------------------------------------------------------------*/

/*
 * tableau qui stocke les taches
 * indice = numero de tache
 * valeur = tache suivante
 */
static uint16_t _file[MAX_PRIO][MAX_TACHES_FILE];

/*
 * index de queue
 * valeur de l'index de la tache en cours d'execution
 * pointe sur la prochaine tache a activer
 */
static uint16_t _queue[MAX_TACHES_FILE];

/*
 * initialise la file
 * entre  : sans
 * sortie : sans
 * description : la queue est initialisee à une valeur de tache impossible
 */
void file_init(void) {
	uint16_t i;

	for (i=0; i<MAX_TACHES_FILE; i++) {
		_queue[i] = F_VIDE;
	}
}

/*
 * ajoute une tache dans la file
 * entre  : n numero de la tache a ajouter
 * sortie : sans
 * description : ajoute la tache n en fin de file
 */
void file_ajoute(uint16_t n) {
	uint16_t num_file, num_t, *q, *f;

	num_file = (n >> 3);
	num_t = n & 7;
	q = &_queue[num_file];
	f = &_file[num_file][0];
    if (*q == F_VIDE) {
        f[num_t] = num_t;
    } else {
        f[num_t] = f[*q];
        f[*q] = num_t;
    }

    *q = num_t;
}

/*
 * retire une tache de la file
 * entre  : t numero de la tache a retirer
 * sortie : sans
 * description : retire la tache t de la file. L'ordre de la file n'est pas
                 modifie
 */
void file_retire(uint16_t t) {
	uint16_t num_file, num_t, *q, *f;

	num_file = t >> 3;
	num_t    = t & 7;
	q = &_queue[num_file];
	f = &_file[num_file][0];

    if (*q == (f[*q])) {
        *q = F_VIDE;
    } else {
        if (num_t == *q) {
            *q = f[*q];
            while (f[*q] != num_t) {
                *q = f[*q];
            }
            f[*q] = f[num_t];
        } else {
            while (f[*q] != num_t) {
                *q = f[*q];
            }
            f[*q] = f[f[*q]];
        }
    }
}

/*
 * recherche la tache suivante a executer
 * entre  : sans
 * sortie : numero de la tache a activer
 * description : queue pointe sur la tache suivante
 */
uint16_t file_suivant(void) {
	uint16_t prio;
	uint16_t id;

	for (prio = 0; prio < MAX_TACHES_FILE; ++prio) {
		if (_queue[prio] != F_VIDE) {
			id = _file[prio][_queue[prio]];
			_queue[prio] = id;
			return (id | prio << 3);
		}
	}

    return (MAX_TACHES_NOYAU);
}

/*
 * affiche la queue, donc la derniere tache
 * entre  : sans
 * sortie : sans
 * description : affiche la valeur de queue
 */
void file_affiche_queue() {
	uint16_t i;
	for (i=0; i < MAX_TACHES_FILE; i++){
		 printf("_queue[%d] = %d\n", i, _queue[i]);
	}
}

/*
 * affiche la file
 * entre  : sans
 * sortie : sans
 * description : affiche les valeurs de la file
 */
void file_affiche() {
	uint16_t i,j;

    for (j=0; j < MAX_TACHES_FILE; j++){
		printf("Tache   | ");
		for (i = 0; i < MAX_TACHES_FILE; i++) {
			printf("%03d | ", i);
		}

		printf("\nSuivant | ");
		for (i = 0; i < MAX_TACHES_FILE; i++) {
			printf("%03d | ", _file[j][i]);
		}
		printf("\n");
    }
}
