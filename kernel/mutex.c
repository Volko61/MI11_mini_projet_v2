/*----------------------------------------------------------------------------*
 * fichier : mutex.c                                                            *
 * gestion des mutex pour le mini-noyau temps reel                       *
 *----------------------------------------------------------------------------*/

#include "mutex.h"

#include "fifo.h"
#include "noyau_prio.h"
#include <stdio.h>

#define NO_OWNER_TASK_ID -1

/*----------------------------------------------------------------------------*
 * declaration des structures                                                 *
 *----------------------------------------------------------------------------*/

/*
 * structure definissant un mutex
 */
typedef struct {
    FIFO wait_queue;	// File d'attente des taches qui veulent prendre ce mutex
    uint8_t owner_id;    // ID de la tâche qui détient le mutex. NO_OWNER_TASK_ID si libre.
    int8_t ref_count;    // Compteur de références. -1 si non créer (et donc non libre), 0 si creer et dispo, >0 si acquis.
} MUTEX;

/*----------------------------------------------------------------------------*
 * variables globales internes                                                *
 *----------------------------------------------------------------------------*/

/*
 * variable stockant tous les mutex du systeme
 */
MUTEX _mutex[MAX_MUTEX];

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
 * initialise les mutex du systeme
 * entre  : sans
 * sortie : sans
 * description : initialise le tableau des mutex de telle manière qu'ils soient tous disponibles
 */
void m_init(void) {
	register MUTEX *m = _mutex;
	register unsigned j;

	for (j = 0; j < MAX_MUTEX; j++)
	{
		m->owner_id = NO_OWNER_TASK_ID;
		m->ref_count = -1;
		m++;
	}
}

/*
 * cree un mutex
 * entre  : sans
 * sortie : Retourne le numéro de mutex si ok, MAX_MUTEX sinon.
 * description : Crée un mutex.
 */
uint8_t m_create() {
	register unsigned n = 0;
	register MUTEX *m = &_mutex[n];

	_lock_();
	/* Rechercher un mutex libre */
	while(m->ref_count != -1 && n < MAX_MUTEX)
	{
		n++;
		m = &_mutex[n];
	}
	if (n < MAX_MUTEX)
	{
		// intiialise une file d'attente de toutes les taches cherchant à obtenir ce mutex
		fifo_init(&(m->wait_queue)); 
		m->ref_count = 0;
		m->owner_id = NO_OWNER_TASK_ID;
	}
	else
	{
		n = MAX_MUTEX;
		printf("Volonté de creer un mutex mais le tableau est plein");
	}
	_unlock_();

	return n;
}


/*
 * Acquiert le mutex n.
 * entre  : numero du mutex a prendre
 * sortie : sans
 * description :
 * 		Acquiert le mutex n. Les mutex sont ré-entrants :
 * 		si une tâche ré-acquiert un mutex dont elle est
 * 		déjà propriétaire, ce n'est pas une erreur,
 * 		elle ne dormira pas dessus
 */
void m_acquire(uint8_t n) {
	if(n < 0 || n >= MAX_MUTEX ){
		printf("L'index du mutex n'est pas valide");
		noyau_exit();
	}

	register MUTEX *m = &_mutex[n];

	if(m->ref_count==-1){
		printf("Le mutex n'as pas encore été créer m_aquaire");
		noyau_exit();
	}

	_lock_();
	// Check si mutex bien deja créer (sinon erreur donc crash system)
	if (m->ref_count == -1){ // supprimer redondonce 
		_unlock_();
		printf("Volonté d'aquérir un mutex meme pas créer");
		noyau_exit();
	}

	// Si non libre
	if (m->ref_count != 0){
		// A part si on est justement le propriétaire (aucquel cas on augmente notre ref_count)
		if(m->owner_id == noyau_get_tc()){ 
			// si la tache courante est en possession du mutex et le reprend
			m->ref_count++;

		}else{
			// Alors c'est qu'il faut attendre qu'il se libère
			fifo_ajoute(&(m->wait_queue), noyau_get_tc());
			dort();

		}
	}else{
		// Si libre
		m->owner_id = noyau_get_tc();
		m->ref_count++;
	}
	_unlock_();
	return;
}

/*
 * Libere le mutex n.
 * entre  : numero du mutex a liberer
 * sortie : sans
 * description :
 * 		Libere le mutex n.
 */
void m_release(uint8_t n) {
	if(n < 0 || n >= MAX_MUTEX ){
		printf("L'index du mutex n'est pas valide");
		noyau_exit();
	}
	register MUTEX *m = &_mutex[n];
	uint8_t* new_task;

	if(m->ref_count==-1){
		printf("Le mutex n'as pas encore été créer m_release");
		noyau_exit();
	}

	if(m->owner_id!=noyau_get_tc()){
		printf("Le processus courant (%d) ne détient pas le mutex (owner : %d)", noyau_get_tc(), m->owner_id);
		noyau_exit();
	}


	_lock_();

	m->ref_count--;
	// Si la tache a completement fini d'utiliser la ressource
	if(m->ref_count == 0){
		// Si il y a des taches en attente à lancer
		if(m->wait_queue.fifo_taille > 0){
			if(fifo_retire(&(m->wait_queue), new_task)== 0) {
				printf("Erreur du fifo_retire de m_relase: possible que ce soit Tache suivant peut etre indisponible");
				m->owner_id = NO_OWNER_TASK_ID;
			}else{
				m->owner_id = *new_task;
				m->ref_count = 0;
				printf("reveille : %d\n", (uint16_t) *new_task);
				printf("reveille : %d\n", *new_task);
				reveille((uint16_t) *new_task);
			}
		}
	}

	_unlock_();
}


/*
 * ferme un mutex pour qu'il puisse etre reutilise
 * entre  : numero du mutex a fermer
 * sortie : sans
 * description : ferme un mutex
 *               en cas d'erreur, le noyau doit etre arrete
 */
void m_destroy(uint8_t n) {
	if(n < 0 || n >= MAX_MUTEX ){
		printf("L'index du mutex n'est pas valide");
		noyau_exit();
	}
	register MUTEX *m = &_mutex[n];

	if(m->ref_count==-1){
		printf("Le mutex n'as pas encore été créer m_destroy");
		noyau_exit();
	}
	if(m->ref_count>0){
		printf("Le mutex est encore détenu par une tache (%d)", m->owner_id);
		noyau_exit();
	}

	_lock_();


	 fifo_init(&(m->wait_queue));
	 m->ref_count=-1;
	 m->owner_id=NO_OWNER_TASK_ID;
	_unlock_();
}


