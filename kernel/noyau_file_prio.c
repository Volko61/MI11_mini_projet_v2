/*----------------------------------------------------------------------------*
 * fichier : noyau_file_prio.c
 *
 * Description :
 * Gestion de la file d'attente des tâches prêtes et actives avec gestion
 * des identifiants de tâches explicites pour permettre l'héritage de priorité.
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include "noyau_file_prio.h"
// recuperation du bon fichier selon l'architecture pour la fonction printf
#include "../io/serialio.h"


/*----------------------------------------------------------------------------*
 * NOUVELLES STRUCTURES DE DONNÉES
 *----------------------------------------------------------------------------*/

// Structure pour un élément dans la file de l'ordonnanceur
typedef struct {
    uint16_t suivant;  // Index de la tâche suivante dans la même file de priorité
    uint16_t id_tache; // Identifiant explicite et permanent de la tâche
} TacheFile;

// Structure pour stocker la position d'une tâche (pour une recherche rapide)
typedef struct {
    uint16_t prio;
    uint16_t index;
} PosTache;


/*----------------------------------------------------------------------------*
 * variables communes a toutes les procedures
 *----------------------------------------------------------------------------*/

/*
 * NOUVEAU : Tableau qui stocke les tâches.
 * Chaque élément contient l'index du suivant et l'ID explicite de la tâche.
 */
static TacheFile _file[MAX_PRIO][MAX_TACHES_FILE];

/*
 * NOUVEAU : Tableau de recherche inversée pour trouver rapidement une tâche par son ID.
 * L'index du tableau est l'ID de la tâche.
 * La valeur est la position (priorité, index) dans le tableau _file.
 * NOTE : MAX_TACHES_NOYAU doit être défini et assez grand pour contenir tous les ID de tâches possibles.
 */
#define MAX_TACHES_NOYAU 64 // Exemple de valeur, à adapter
static PosTache _tache_pos[MAX_TACHES_NOYAU];

/*
 * index de queue (pointe sur la tâche "courante" de chaque niveau de priorité)
 */
static uint16_t _queue[MAX_PRIO];

/*
 * initialise la file
 */
void file_init(void) {
    uint16_t i, j;

    for (i = 0; i < MAX_PRIO; i++) {
        _queue[i] = F_VIDE;
        for (j = 0; j < MAX_TACHES_FILE; j++) {
            // Initialise l'ID de la tâche à F_VIDE pour marquer le slot comme libre
            _file[i][j].id_tache = F_VIDE;
            _file[i][j].suivant = F_VIDE; // Optionnel mais propre
        }
    }
     // Initialise également la table de position
    for (i = 0; i < MAX_TACHES_NOYAU; i++) {
        _tache_pos[i].prio = F_VIDE;
        _tache_pos[i].index = F_VIDE;
    }
}

/*
 * MODIFIÉ : ajoute une tache dans la file
 * entre  : n -> identifiant de position (prio | index)
 * id_tache -> identifiant explicite de la tâche
 */
void file_ajoute(uint16_t n, uint16_t id_tache) {
    uint16_t num_file, num_t;
    uint16_t *q;
    TacheFile *f;

    num_file = (n >> 3); // Priorité
    num_t = n & 7;       // Index dans la file de priorité
    q = &_queue[num_file];
    f = &_file[num_file][0];

    // Stocke l'identifiant explicite et sa position
    f[num_t].id_tache = id_tache;
    _tache_pos[id_tache].prio = num_file;
    _tache_pos[id_tache].index = num_t;

    if (*q == F_VIDE) {
        f[num_t].suivant = num_t;
    } else {
        f[num_t].suivant = f[*q].suivant;
        f[*q].suivant = num_t;
    }

    *q = num_t;
}

/*
 * retire une tache de la file
 * entre  : t numero de la tache a retirer (identifiant de position)
 */
void file_retire(uint16_t t) {
    uint16_t num_file, num_t, current_q, id_a_retirer;
    uint16_t *q;
    TacheFile *f;

    num_file = t >> 3;
    num_t = t & 7;
    q = &_queue[num_file];
    f = &_file[num_file][0];

    // Si la file est vide, ne rien faire
    if (*q == F_VIDE) return;
    
    id_a_retirer = f[num_t].id_tache;

    // Cas où il n'y a qu'un seul élément
    if (*q == f[*q].suivant && *q == num_t) {
        *q = F_VIDE;
    } else {
        current_q = *q;
        // Trouve le prédécesseur de la tâche à retirer
        while (f[current_q].suivant != num_t) {
            current_q = f[current_q].suivant;
        }
        // Le prédécesseur pointe maintenant sur le successeur de la tâche retirée
        f[current_q].suivant = f[num_t].suivant;
        
        // Si on retire la tâche courante, il faut mettre à jour la queue
        if (*q == num_t) {
            *q = current_q;
        }
    }
    
    // Libère le slot dans _file et dans _tache_pos
    f[num_t].id_tache = F_VIDE;
    if(id_a_retirer != F_VIDE) {
        _tache_pos[id_a_retirer].prio = F_VIDE;
        _tache_pos[id_a_retirer].index = F_VIDE;
    }
}


/*
 * NOUVEAU : Échange les identités de deux tâches dans les files.
 * Cela a pour effet de permuter leur priorité et leur position dans le tourniquet.
 * entre : id1 -> identifiant explicite de la tâche 1
 * id2 -> identifiant explicite de la tâche 2
 */
void file_echange(uint16_t id1, uint16_t id2) {
    PosTache pos1, pos2;
    uint16_t temp_id;

    // 1. Récupérer les positions des tâches via le tableau de recherche
    pos1 = _tache_pos[id1];
    pos2 = _tache_pos[id2];

    // Vérifier si les tâches existent bien dans les files
    if (pos1.prio == F_VIDE || pos2.prio == F_VIDE) {
        // Gérer l'erreur, par exemple :
        // printf("Erreur: Tache %d ou %d non trouvee pour echange.\n", id1, id2);
        return;
    }

    // 2. Permuter les identifiants de tâches dans le tableau _file
    temp_id = _file[pos1.prio][pos1.index].id_tache;
    _file[pos1.prio][pos1.index].id_tache = _file[pos2.prio][pos2.index].id_tache;
    _file[pos2.prio][pos2.index].id_tache = temp_id;

    // 3. Mettre à jour le tableau de recherche inversée
    _tache_pos[id1] = pos2;
    _tache_pos[id2] = pos1;
}


/*
 * MODIFIÉ : recherche la tache suivante a executer
 * sortie : identifiant explicite de la tache a activer
 */
uint16_t file_suivant(void) {
    uint16_t prio;
    uint16_t index_tache;

    for (prio = 0; prio < MAX_PRIO; ++prio) {
        if (_queue[prio] != F_VIDE) {
            // La tâche à exécuter est la suivante de la tâche courante
            index_tache = _file[prio][_queue[prio]].suivant;
            // La nouvelle tâche courante devient celle qu'on vient de sélectionner
            _queue[prio] = index_tache;
            // Retourne l'identifiant EXPLICITE de la tâche
            return _file[prio][index_tache].id_tache;
        }
    }
    // Si aucune tâche n'est prête
    return (MAX_TACHES_NOYAU); // ou une autre valeur d'erreur
}

/*
 * affiche la queue (pour le debug)
 */
void file_affiche_queue() {
    uint16_t i;
    for (i = 0; i < MAX_PRIO; i++) {
        printf("_queue[%d] = %d\n", i, _queue[i]);
    }
}

/*
 * affiche la file (pour le debug)
 */
void file_affiche() {
    uint16_t i, j;

    for (j = 0; j < MAX_PRIO; j++) {
        printf("File Prio %d (queue a %d):\n", j, _queue[j]);
        printf("Index   | ");
        for (i = 0; i < MAX_TACHES_FILE; i++) {
            printf("%03d | ", i);
        }
        printf("\nID Tache| ");
        for (i = 0; i < MAX_TACHES_FILE; i++) {
            if (_file[j][i].id_tache == F_VIDE) printf("--- | ");
            else printf("%03d | ", _file[j][i].id_tache);
        }
        printf("\nSuivant | ");
        for (i = 0; i < MAX_TACHES_FILE; i++) {
             if (_file[j][i].suivant == F_VIDE) printf("--- | ");
            else printf("%03d | ", _file[j][i].suivant);
        }
        printf("\n------------------------------------------------------------------\n");
    }
}