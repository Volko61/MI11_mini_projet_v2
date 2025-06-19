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
static uint16_t _file[MAX_PRIO][MAX_TACHES_FILE]; //_file[n] = [p0, p3, p4,...,pk] contains all taks of priority of n ordered

uint16_t _id[MAX_PRIO][MAX_TACHES_FILE];  // _id[prio_task(3 fist bites)][task_number(3 last bites)] = complete_ID 

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
		for (int j = 0; j < MAX_TACHES_FILE; j++) {
			// initialise des tableaux vides
            _file[i][j] = F_VIDE;
            _id[i][j] = F_VIDE; 
        }
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
	q = &_queue[num_file];  // queue de la file de priorité associées à la tache
	f = &_file[num_file][0];  // récupère le prointeur sur la preimère position de la file
	uint16_t *id = &_id[num_file][0];

    if (*q == F_VIDE) {
        f[num_t] = num_t; // la tache pointe sur elle même, elle est la queue et la tete
    } else {
			f[num_t] = f[*q]; // la nouvelle tache devient queue et pointe sur la tête
			f[*q] = num_t; // l'ancienne queue pointe sur la nouvelle tache ajoutée
		}
	
	id[num_t] = n;
    *q = num_t;
}

/*
 * retire une tache de la file
 * entre  : t numero de la tache a retirer
 * sortie : sans
 * description : retire la tache t de la file. L'ordre de la file n'est pas
                 modifie
 */
// void file_retire(uint16_t t) {
//     uint16_t num_file = t >> 3;
//     uint16_t num_t = t & 7;
//     uint16_t *q = &_queue[num_file];
//     uint16_t *f = &_file[num_file][0];
//     uint16_t *id = &_id[num_file][0];

//     if (*q == F_VIDE) {
//         return; // File vide, rien à faire
//     }

//     if (*q == num_t) {
//         *q = f[*q];
//         id[num_t] = F_VIDE; // Effacer l'identifiant
//         if (*q == num_t) {
//             *q = F_VIDE; // Dernière tâche retirée
//         }
//     } else {
//         uint16_t prev = *q;
//         while (f[prev] != num_t && f[prev] != F_VIDE) {
//             prev = f[prev];
//         }
//         if (f[prev] == num_t) {
//             f[prev] = f[num_t];
//             id[num_t] = F_VIDE; // Effacer l'identifiant
//         }
//     }
// }

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
// uint16_t file_suivant(void) {
//     uint16_t prio;
//     for (prio = 0; prio < MAX_PRIO; ++prio) {
//         if (_queue[prio] != F_VIDE) {
//             uint16_t id = _id[prio][_queue[prio]];
// 			_queue[prio] = _file[prio][_queue[prio]];
//             return id; // Retourner l'identifiant explicite
//         }
//     }
//     return MAX_TACHES_NOYAU;
// }

uint16_t file_suivant(void) {
	uint16_t prio;
	uint16_t id;
	uint16_t explicite_id;

	for (prio = 0; prio < MAX_TACHES_FILE; ++prio) {
		if (_queue[prio] != F_VIDE) {
			id = _file[prio][_queue[prio]]; //retourne la tête de file la position dans la fille
			_queue[prio] = id; // la tête devient la nouvelle queue 
			explicite_id = _id[prio][id];
			printf("NEXT TASK SUIV : %d", explicite_id);
			return explicite_id;
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
    uint16_t i, j;
    for (j = 0; j < MAX_PRIO; j++) {
        printf("Priorité %d\n", j);
        printf("Tache   | ");
        for (i = 0; i < MAX_TACHES_FILE; i++) {
            printf("%03d | ", i);
        }
        printf("\nSuivant | ");
        for (i = 0; i < MAX_TACHES_FILE; i++) {
            printf("%03d | ", _file[j][i]);
        }
        printf("\nID      | ");
        for (i = 0; i < MAX_TACHES_FILE; i++) {
            printf("%03d | ", _id[j][i]);
        }
        printf("\n\n");
    }
}


void file_echange(uint16_t id1, uint16_t id2) { // s'assurer que les id passés en paramètres sont bien explicites et non des num de tache
		//printf("J ECHANGE MES DEUX TACHES \n");
		printf("ID TACHE ATTENTE : %d ID TACHE MUTEX : %d \n", id1, id2);
		uint16_t prio1 = id1 >> 3;
    uint16_t num_t1 = id1 & 7;
    uint16_t prio2 = id2 >> 3;
    uint16_t num_t2 = id2 & 7;

    uint16_t *file_p1 = &_id[prio1][0]; // recupère la liste des taches de cette prio
    uint16_t *file_p2 = &_id[prio2][0];

    // Échanger les identifiants dans les tableaux _id
    uint16_t temp = file_p1[num_t1];  // récupère l'id explicite de la tache 1
    file_p1[num_t1] = file_p2[num_t2];
    file_p2[num_t2] = temp;
}
