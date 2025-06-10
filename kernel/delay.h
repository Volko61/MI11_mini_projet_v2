/*
 * delay.h
 *
 *  Created on: 28 mai 2020
 *      Author: jdm
 */

#ifndef __DELAY_H__
#define __DELAY_H__

/*----------------------------------------------------------------------------*
 * fonctions de gestion des délais                                            *
 *----------------------------------------------------------------------------*/
/*
 * entrée  : nombre de tick d'attente
 * sortie  : sans
 * description : renseigne le compteur de délai de la structure _contexte de la tâche courante
 * 				 endort une tâche pour un nombre fini de ticks noyau
 */
void delay(uint32_t nticks);

/*
 * entrée  : sans (fonction appelée dans task_switch)
 * sortie : sans
 * description : parcours l'ensemble des contextes de tâches pour vérifier celles qui sont SUSP
 * 				 pour celles dont le compteur est non nul
 * 				 	décrémente le compteur
 * 				 	remet la tache en exécution si son compteur arrive à zéro
 *
 */
void delay_process(void);

#endif //__DELAY_H__








