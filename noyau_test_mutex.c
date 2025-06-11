/*----------------------------------------------------------------------------*
 * fichier : noyau_test.c                                                     *
 * programme de test du noyaut                                                *
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

#include "hwsupport/stm_uart.h"
#include "kernel/noyau_prio.h"
#include "kernel/delay.h"
#include "io/serialio.h"
#include "io/TERMINAL.h"
#include "kernel/mutex.h"

uint8_t mutex;


/* Structure TACHE_PARAM
 *
 * Paramètre des tâches
 * start_delay est le nombre de ticks que la tâche doit attendre avant de commencer
 * un nouveau travail
 *
 * work_to_do est la quantité de travail à faire
 *
 * ATTENTION : sous QEMU, la présence de points d'arrêts dans le programme ralentit
 * énormément l'émulateur, il comptera donc beaucoup moins vite dans ce cas.
 *
 */
typedef struct {
	uint16_t start_delay;
	int work_to_do;
} TACHE_PARAM;

/* tacheMutex
 *
 * Cette tâche générique cherche à acquérir le mutex. Une fois acquis, elle simule un travail.
 *
 */
TACHE tacheMutex(void *arg)
{
	TACHE_PARAM *p = (TACHE_PARAM *) arg;

	while(1) {
		delay(p->start_delay);
		m_acquire(mutex);
		for (volatile int i = 0; i < p->work_to_do; i++) continue;
		m_release(mutex);
	}
}

/* tacheAutre
 *
 * Cette tâche générique simule un travail sans synchronisation
 *
 */
TACHE tacheAutre(void *arg)
{
	TACHE_PARAM *p = (TACHE_PARAM *) arg;

	while(1) {
		delay(p->start_delay);
		for (volatile int i = 0; i < p->work_to_do; i++) continue;
	}
}

/* tachedefond
 *
 * Cette tâche crée les autres. Elle met ensuite fin à l'application si une touche est pressée.
 *
 */
TACHE	tachedefond(void *arg)
{
	/* Paramètres des tâches */
	TACHE_PARAM params[3] = {{24, 200000000}, {28, 400000000}, {20, 800000000}};

	SET_CURSOR_POSITION(3,1);
	puts("------> EXEC tache de fond");

	/* Création de trois tâches :
	 * - Deux tâches qui partagent une section critique au travers du mutex
	 * - Une tâche de priorité intermédiaire sans mutex.
	 */
	active(cree(tacheMutex, 2, (void*) &params[0]));
	active(cree(tacheAutre, 4,  (void*) &params[1]));
	active(cree(tacheMutex, 6,  (void*) &params[2]));

	while(1);
	//usart_read();
	//printf("Touche trouvée, quittons");
	noyau_exit();
}

int main()
{
	usart_init(115200);
	CLEAR_SCREEN(1);
	printf("KK");
	puts("PROUTV3");
	puts("Test noyau");
    puts("Noyau preemptif");
    puts("dd");

    m_init();
    mutex = m_create();
	start(tachedefond);
  return(0);
}
