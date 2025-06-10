/* NOYAU_PRIO.C */
/*--------------------------------------------------------------------------*
 *               Code C du noyau preemptif qui tourne sur ARM                *
 *                                 NOYAU.C                                  *
 *--------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

#include "../hwsupport/stm32h7xx.h"
#include "../io/serialio.h"
#include "noyau_prio.h"
#include "noyau_file_prio.h"
#include "delay.h"
#include "chronogram.h"


/*--------------------------------------------------------------------------*
 *           Constantes pour la création du contexte CPU initial            *
 *--------------------------------------------------------------------------*/
#define THUMB_ADDRESS_MASK (0xfffffffe) /* Masque pointeurs de code Thumb   */
#define TASK_EXC_RETURN (0xfffffff9)    /* Valeur de retour d'exception :   */
                                        /* mspnamed active, mode Thread          */
#define TASK_PSR (0x01000000)           /* PSR initial                      */

/*--------------------------------------------------------------------------*
 *            Variables internes du noyau                                   *
 *--------------------------------------------------------------------------*/
static int compteurs[MAX_TACHES_NOYAU];  /* Compteurs d'activations               */
NOYAU_TCB  _noyau_tcb[MAX_TACHES_NOYAU]; /* tableau des contextes                 */
volatile uint16_t _tache_c;        /* numéro de tache courante              */
uint32_t _tos;                     /* adresse du sommet de pile des tâches  */
uint8_t _timer_event = 0;          /* variable de détection d'appel SYSTICK */

/*----------------------------------------------------------------------------*
 * fonctions du noyau                                                         *
 *----------------------------------------------------------------------------*/

/*
 * termine l'execution du noyau
 * entre  : sans
 * sortie : sans
 * description : termine l'execution du noyau, execute en boucle une
 *               instruction vide
 */
 void noyau_exit(void) {
    uint16_t j;
    /* Q2.1 : desactivation des interruptions */
    _irq_disable_();                       
    
    /* affichage du nombre d'activation de chaque tache !*/
    printf("Sortie du noyau\n");
    for (j = 0; j < MAX_TACHES_NOYAU; j++) {
        printf("\nActivations tache %d : %d", j, compteurs[j]);
    }
    /* Q2.2 : Que faire quand on termine l'execution du noyau ? */
    for (;;) continue;          /* Terminer l'exécution                     */
}

/*
 * termine l'execution d'une tache
 * entre  : sans
 * sortie : sans
 * description : une tache dont l'execution se termine n'est plus executee
 *               par le noyau
 *               cette fonction doit etre appelee a la fin de chaque tache
 */
 void fin_tache(void) {
    /* Q2.3 :             Début section critique                             */
    _lock_();      
    /* Q2.4 : la tache est enlevee de la file des taches et ... ?  */
    /* 4 instructions */
    _noyau_tcb[_tache_c].status = CREE;
    file_retire(_tache_c);      /* la tache est enlevee de la file          */
    schedule();                 /* Activation d'une tâche prête            */
    _unlock_();                 /* Fin section critique                     */
}

/*
 * demarrage du system en mode multitache
 * entre  : adresse de la tache a lancer
 * sortie : sans
 * description : active la tache et lance le scheduler
 */
 void start(TACHE_ADR adr_tache) {
    uint16_t j;
    register unsigned int sp asm("sp");

    /* Q2.5 : initialisation de l'etat des taches                            */
    for (j = 0; j < MAX_TACHES_NOYAU; j++) {
        _noyau_tcb[j].status = NCREE; /* initialisation de l'etat des taches */
    }
    /* Q2.6 : initialisation de la tache courante */
    _tache_c = 56;
    /* initialisation de la file circulaire de gestion des tâches           */ 
    file_init();                 
    /* Q2.7 : initialisation de la variable _tos sommet de la pile           */
    /* Haut de la pile des tâches avec réservation  pour le noyau  */
    _tos = sp - PILE_NOYAU;          /* Haut de la pile des tâches          */

    /* Q2.8 : on interdit les interruptions  */
    _irq_disable_();             
    /* Q2.9 : initialisation du timer system a 100 Hz   (voir cortex.c)    */
    systick_start(CORE_CLK / 10);
    /* Q2.10 : initialisation de l'interruption systick  (voir cortex.c) */
    systick_irq_enable();
    /* Q2.11 : creation et activation de la premiere tache                          */
    active(cree(adr_tache, 7, 0));
    /* Q2.12 : on autorise les interruptions */
    _irq_enable_();
}

/*
 * creation d'une nouvelle tache
 * entre  : adresse de la tache a creer
 * sortie : numero de la tache cree
 * description : la tache est creee en lui allouant une pile et un numero
 *               en cas d'erreur, le noyau doit etre arrete
 * Err. fatale: priorite erronnee, depassement du nb. maximal de taches 
 */
 // Doit être déclaré au niveau du fichier, avec _noyau_tcb
// Ce tableau garde une trace du prochain index libre pour chaque niveau de priorité.
static uint16_t _prochain_index_prio[MAX_PRIO] = {0};

/**
 * @brief Crée une nouvelle tâche dans le système.
 *
 * @param adr_tache Pointeur vers la fonction de la tâche.
 * @param prio      Priorité de la tâche (0 = plus haute).
 * @param arg       Argument à passer à la tâche.
 * @return          L'identifiant permanent et unique de la tâche créée, ou une valeur d'erreur.
 */
uint16_t cree(TACHE_ADR adr_tache, uint16_t prio, void* arg) {

    uint16_t id_permanent;    // L'ID unique de la tâche (son index dans _noyau_tcb)
    uint16_t id_position;     // L'ID de son slot dans la file (prio | index)
    uint16_t index_tache;     // L'index dans la file de priorité (0 à 7)
    NOYAU_TCB *p_tcb;

    _lock_(); // Début de la section critique

    // --- 1. Trouver un TCB libre pour assigner un ID permanent ---
    for (id_permanent = 0; id_permanent < MAX_TACHES_NOYAU; id_permanent++) {
        if (_noyau_tcb[id_permanent].status == NCREE) {
            break; // TCB libre trouvé à l'index 'id_permanent'
        }
    }

    // Vérifier si un TCB a été trouvé
    if (id_permanent >= MAX_TACHES_NOYAU) {
        printf("Erreur: Plus de TCB disponibles.\n");
        _unlock_();
        noyau_exit(); // Ou retourner un code d'erreur
    }

    // --- 2. Allouer un slot de position dans la file de priorité demandée ---
    if (prio >= MAX_PRIO) {
        printf("Erreur: Priorite %d invalide.\n", prio);
        _unlock_();
        noyau_exit();
    }

    index_tache = _prochain_index_prio[prio]++;
    if (index_tache >= MAX_TACHES_FILE) {
        printf("Erreur: Plus de slots disponibles pour la priorite %d.\n", prio);
        _unlock_();
        noyau_exit();
    }

    // On construit l'identifiant de position
    id_position = index_tache | (prio << 3);

    // --- 3. Initialiser le TCB ---
    p_tcb = &_noyau_tcb[id_permanent];

    // Allocation de la pile
    p_tcb->sp_ini = _tos;
    _tos -= PILE_TACHE;
    
    // Vérification du débordement de pile global
    if (_tos <= _bos) {
        printf("Erreur: Debordement de la pile systeme.\n");
        _unlock_();
        noyau_exit();
    }

    // Initialisation des champs du TCB
    p_tcb->task_adr = adr_tache;
    p_tcb->arg = arg;
    p_tcb->delay = 0;
    
    // NOUVEAU : On sauvegarde l'identifiant de position original de la tâche.
    // C'est crucial pour savoir où la réinsérer après un `dort()` ou `delay()`.
    p_tcb->id_position = id_position;

    // La tâche est maintenant créée mais pas encore prête à être exécutée.
    p_tcb->status = CREE;

    _unlock_(); // Fin de la section critique

    // On retourne l'identifiant PERMANENT de la tâche
    return id_permanent;
}
/*
 * ajout d'une tache pouvant etre executee a la liste des taches eligibles
 * entre  : numero de la tache a ajouter
 * sortie : sans
 * description : ajouter la tache dans la file d'attente des taches eligibles
 *               en cas d'erreur, le noyau doit etre arrete
 */
 void active(uint16_t tache){
    NOYAU_TCB *p = &_noyau_tcb[tache]; /* acces au contexte tache             */

    /* Q2.22 : verifie que la tache n'est pas dans l'etat NCREE, sinon arrete le noyau*/
    if (p->status == NCREE) {
    	printf("Tnetative de rendre active une tache non creer");
        noyau_exit();               /* sortie du noyau                      */
    }

    /* Q2.23 : debut section critique */
    _lock_();        

    /* Q2.24 : activation de la tache seulement si elle est a l'état CREE    */
    if (p->status == CREE) {                    /* n'active que si receptif  */
        /* Créer un contexte initial en haut de la pile de la nouvelle tâche */
        p->sp_start = p->sp_ini - sizeof(CONTEXTE_CPU_BASE); /* Réserver la Place pour 
                                                                le contexte sur la pile de la tâche */
        p->sp_start &= 0xfffffff8;               /* Aligner au multiple de 8 inférieur   */
		CONTEXTE_CPU_BASE *c = (CONTEXTE_CPU_BASE*) p->sp_start;
        c->pc = ((uint32_t) p->task_adr) & THUMB_ADDRESS_MASK; /* Adresse de la tâche dans pc, attention au bit de poids faible*/
        c->lr = ((uint32_t) fin_tache);         /* Adresse de retour de la tâche dans lr */
        c->lr_exc = TASK_EXC_RETURN;            /* veleur initiale de retour d'exeption dans lr_exc */
        c->psr = TASK_PSR;                      /* veleur initiale des flags dans psr    */
        c->r0 = (uint32_t) p->arg;				/* valeur de l'argument */
         
        p->status = PRET;           			/* changement d'etat, mise a l'etat PRET */
        file_ajoute(tache);              		/* ajouter la tache dans la liste        */
        schedule();                 			/* activation d'une tache prete          */
    }
    /* Q2.25 : fin section critique */
    _unlock_();                    
}

/*--------------------------------------------------------------------------*
 *                  ORDONNANCEUR preemptif optimise                         *
 * Entrée : pointeur de contexte CPU de la tâche courante                   *
 * Sortie : pointeur de contexte CPU de la nouvelle tâche courante          *
 * Descrip: sauvegarde le pointeur de contexte de la tâche courante,        *
 *      Élit une nouvelle tâche et retourne son pointeur de contexte.       *
 *      Si aucune tâche n'est éligible, termine l'exécution du noyau.       *
 *--------------------------------------------------------------------------*/
uint32_t task_switch(uint32_t sp)
{
    NOYAU_TCB *p = &_noyau_tcb[_tache_c]; /* acces au contexte tache courante */
    char sep;

    /* Q2.26 : sauvegarde du pointeur sur le contexte sauvegardé sur la pile 
       dans le contexte de la tache */
    p->sp = sp;      

    if (_timer_event) {
    	delay_process();
    	sep = '|';
    } else {
    	sep = ' ';
    }

    _timer_event = 0;

    /* on bascule sur la nouvelle tache a executer */
    /* Q2.27 : recherche la prochaine tache a executer */
    _tache_c = file_suivant();
	draw_tick(_tache_c, sep);
    /* Q2.28 : acces contexte suivant                   */
    p = &_noyau_tcb[_tache_c];
    /* Q2.29 : verifie qu'une tache suivante existe, sinon arret du noyau */
    if (_tache_c == MAX_TACHES_NOYAU) {
        printf("Plus rien à ordonnancer.\n");
        noyau_exit();           /* Sortie du noyau                          */
    }

    compteurs[_tache_c]++;      /* MAJ compteur d'activations               */
    /* Q2.30 : retourner la bonne valeur de pointeur de pile 
     Deux cas possible en fonction du statut de la tâche */
    if (p->status == PRET) {
        p->status = EXEC;
        return p->sp_start;
    } else {
        return p->sp;
    }
}

/*--------------------------------------------------------------------------*
 *              --- Gestionnaire d'exception PEND SVC ---                   *
 * Descrip: Appelé lors de l'exception PEND SVC provoquée par pendsv_trigger*
 *      Sauvegarde le contexte CPU sur la pile de la tâche et provoque une  *
 *      commutation de contexte.                                            *
 *------------------------------------------------------------------------- */
void __attribute__((naked)) _pend_svc(void) {
    /* Q2.31 : sauvegarde du complément de conyexte et appel de 
                 l'ordonnaceur */
    __asm__ __volatile__ (
            "push   {r4-r11,lr}\n"  /* Sauvegarder le complément de  contexte    
                                       sur la pile                          */                          
            "mov    r0, sp     \n"  /* r0 = 1er paramètre de task_switch    */
            "bl     task_switch\n"  /* Commutation de contexte              */
            "mov    sp, r0     \n"  /* r0 = valeur de retour de task_switch
                                       dans ? */
            "pop    {r4-r11,lr}\n"  /* Restituer le contexte                */
            "bx     lr         \n"  /* Retour d'exception                   */
    );
}

/*--------------------------------------------------------------------------*
 *                  --- Provoquer une commutation ---                       *
 *                                                                          *
 * Descrip: planifie une exception PEND SVC pour forcer une commutation     *
 *      de tâche                                                            *
 *--------------------------------------------------------------------------*/
void schedule(void)
{
	int isr_num;
	__asm__ __volatile__(
			"mrs %[result], ipsr\n\t"
			: [result] "=r" (isr_num));
	if (isr_num == 15) {
		_timer_event = 1;
	}
    pendsv_trigger();
}



/*-------------------------------------------------------------------------*
 *                    --- Endormir la tâche courante ---                   *
 * Entree : Neant                                                          *
 * Sortie : Neant                                                          *
 * Descrip: Endort la tâche courante et attribue le processeur à la tâche  *
 *          suivante.                                                      *
 *                                                                         *
 * Err. fatale:Neant                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void dort(void) {
    _lock_();
    _noyau_tcb[_tache_c].status = SUSP;
    file_retire(_tache_c);
    schedule();
    _unlock_();
}

/*-------------------------------------------------------------------------*
 *                    --- Réveille une tâche ---                           *
 * Entree : numéro de la tâche à réveiller                                 *
 * Sortie : Neant                                                          *
 * Descrip: Réveille une tâche. Les signaux de réveil ne sont pas mémorisés*
 *                                                                         *
 * Err. fatale:tâche non créée                                             *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void reveille(uint16_t t) {
    NOYAU_TCB *p;

    p = &_noyau_tcb[t];

    if (p->status == NCREE){
    	printf("Tnetative de rendre reveiller une tache non creer : %d\n", t);
        noyau_exit();
    }

    _lock_();
    if (p->status == SUSP) {
        p->status = EXEC;
        file_ajoute(t);
    }
	schedule();
    _unlock_();
}

/*
 * recupere le numero de tache courante
 * entre  : sans
 * sortie : valeur de _tache_c
 * description : fonction d'acces a la valeur d'identite de la tache courante
 */
uint16_t 	noyau_get_tc(void){
	return(_tache_c);
}

NOYAU_TCB* 	noyau_get_p_tcb(uint16_t tcb_nb){
	return &_noyau_tcb[tcb_nb];
}
