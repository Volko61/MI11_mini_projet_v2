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
#include "kernel/sem.h" // Ajout de l'en-tête pour les sémaphores

uint8_t mutex;
uint8_t sem_demarrage; // Sémaphore pour synchroniser le démarrage des tâches


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

  s_wait(sem_demarrage); // Attend le signal de la tâche de fond pour démarrer

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

  s_wait(sem_demarrage); // Attend le signal de la tâche de fond pour démarrer

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
TACHE tachedefond(void *arg)
{
  /* Paramètres des tâches */
  TACHE_PARAM params[3] = {{24, 200000000}, {28, 400000000}, {20, 800000000}};

  SET_CURSOR_POSITION(3,1);
  puts("------> EXEC tache de fond");

  /* Creation du semaphore de démarrage avec une valeur initiale de 0 pour bloquer les tâches */
  sem_demarrage = s_cree(0);

  /* Création de trois tâches :
   * - Deux tâches qui partagent une section critique au travers du mutex
   * - Une tâche de priorité intermédiaire sans mutex.
   */
  uint16_t tache_mutex1 = cree(tacheMutex, 2, (void*) &params[0]);
  uint16_t tache_autre = cree(tacheAutre, 4,  (void*) &params[1]);
  uint16_t tache_mutex2 = cree(tacheMutex, 6,  (void*) &params[2]);

  active(tache_mutex1);
  active(tache_autre);
  active(tache_mutex2);

  /* Libération des tâches une fois qu'elles sont toutes activées */
  s_signal(sem_demarrage);
  s_signal(sem_demarrage);
  s_signal(sem_demarrage);

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
  s_init(); // Initialisation du système de sémaphores
  mutex = m_create();
  start(tachedefond);
  return(0);
}