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

TACHE	tachedefond(void *);
TACHE	tacheGen(void *);

TACHE	tachedefond(void *)
{
	SET_CURSOR_POSITION(3,1);
	puts("------> EXEC tache de fond");

	active(cree(tacheGen, 0,  (void*) 100));
	active(cree(tacheGen, 1,  (void*) 50));
	active(cree(tacheGen, 2, (void*) 60));
	active(cree(tacheGen, 2, (void*) 40));
  	active(cree(tacheGen, 3, (void*) 15));
  	active(cree(tacheGen, 3, (void*) 15));
  	active(cree(tacheGen, 4, (void*) 10));
  	active(cree(tacheGen, 4, (void*) 10));
  	active(cree(tacheGen, 5, (void*) 5));
	active(cree(tacheGen, 6, (void*) 2));
	active(cree(tacheGen, 7, (void*) 1));

	while (!usart_read()) {
	}
}

TACHE	tacheGen(void *arg)
{
	uint32_t wait_time = (uint32_t) arg;

	// on laisse du temps à la tâche de fond de démarrer toutes les tâches
	delay(20);
	while(1){
		for(int i =0; i < 100000; ++i);
		delay(wait_time);
  	  }
}


int main()
{
	usart_init(115200);
	CLEAR_SCREEN(1);
    puts("Test noyau");
    puts("Noyau preemptif");
	start(tachedefond);
  return(0);
}
