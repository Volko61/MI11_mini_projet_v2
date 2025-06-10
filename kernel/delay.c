/*
 * utils.c
 *
 *  Created on: 28 mai 2020
 *      Author: jdm
 */

#include <stdint.h>
#include <stdlib.h>

#include "noyau_prio.h"
#include "noyau_file_prio.h"
#include "delay.h"

/*----------------------------------------------------------------------------*
 * variables communes a toutes les procedures                                 *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 * fonctions de gestion des délais                                            *
 *----------------------------------------------------------------------------*/

/*
 * entrée  : nombre de tick d'attente
 * sortie  : sans
 * description : renseigne le compteur de délai de la structure _contexte de la tâche courante
 * 				 endort la tâche
 */
void delay(uint32_t nticks){
	uint16_t tachecourante;
	NOYAU_TCB* p_tcb = NULL;

	_lock_();
	tachecourante = noyau_get_tc();
	p_tcb = noyau_get_p_tcb(tachecourante);
	if(nticks !=0){
		p_tcb->delay = nticks;
		dort();
	}
	_unlock_();
}

/*
 * entrée  : sans (fonction appelée dans scheduler)
 * sortie : sans
 * description : parcours l'ensemble des contextes de tâches pour vérifier celles qui sont SUSP
 * 				 pour celles dont le compteur est non nul
 * 				 	décrémente le compteur
 * 				 	remet la tache en exécution si son compteur arrive à zéro
 *
 */
void delay_process(void){
	register uint16_t i;
	register NOYAU_TCB* p_tcb = NULL;

	p_tcb = noyau_get_p_tcb(0);
	for(i=0; i< MAX_TACHES_NOYAU; i++){

		if (p_tcb[i].status == SUSP){
			if (p_tcb[i].delay != 0){
				p_tcb[i].delay--;
				if (p_tcb[i].delay == 0){
					p_tcb[i].status = EXEC;
					file_ajoute(i);
				}
			}
		}
	}
}



