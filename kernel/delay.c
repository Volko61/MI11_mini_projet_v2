/*
 * delay.c
 */

#include <stdint.h>
#include <stdlib.h>

#include "noyau_prio.h"
#include "noyau_file_prio.h"
#include "delay.h"

/* ... (fonction delay() inchangée) ... */
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
 * MODIFIÉ : Gère la décrémentation des délais et réveille les tâches.
 */
void delay_process(void){
	register uint16_t i;
	NOYAU_TCB* p_tcb_base = noyau_get_p_tcb(0);

	// Parcourt toutes les TCB possibles
	for(i = 0; i < MAX_TACHES_NOYAU; i++){ // MAX_TACHES_NOYAU doit être défini dans le noyau

		// Si une tâche est suspendue (potentiellement pour un délai)
		if (p_tcb_base[i].status == SUSP){
			// Et si son compteur de délai est actif
			if (p_tcb_base[i].delay > 0){
				p_tcb_base[i].delay--;
				// Si le délai est écoulé
				if (p_tcb_base[i].delay == 0){
					// La tâche redevient prête
					p_tcb_base[i].status = PRET; // Une tâche réveillée est PRÊTE, pas en EXECUTION
					
					// On la ré-ajoute dans la file de l'ordonnanceur
					// en utilisant son identifiant de position original et son ID explicite (i).
					file_ajoute(p_tcb_base[i].id_position, i);
				}
			}
		}
	}
}