/*
 *TERMINAL.h
 *
 *  Created on: 28 mai 2020
 *      Author: jdm
 */


#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>
#include "../io/serialio.h"

#define POS_FILE 40
#define POS_FIN_FILE 120

/*
 * Definition de chaine de caractères pour construire des codes d'échappement pour le terminal
 * Attention il faut compléter par des éléments
 */
#define CODE_FONT_COLOR "\x1B[38;5;"
#define CODE_BACKGROUND_COLOR "\x1B[48;5;"
#define CODE_RESET_COLOR "\x1B[0m"
#define CODE_BOLD_FONT "\x1B[1m"
#define CODE_UNDERLINE_FONT "\x1B[4m"
#define CODE_REVERSED_FONT "\x1B[7m"
#define CODE_ESCAPE_BASE "\x1B["

/*
 * Définition de macro pour envoi de codes d'échappement directement au terminal
 */
#define SET_FONT_COLOR_256(red,green,blue) \
	printf("%s%dm",CODE_FONT_COLOR, 16 + (red * 36) + (green * 6) + blue);

#define SET_FONT_COLOR(color) \
	printf("%s%dm",CODE_FONT_COLOR, color);

#define SET_BACKGROUND_COLOR_256(red,green,blue) \
	printf("%s%dm",CODE_BACKGROUND_COLOR, 16 + (red * 36) + (green * 6) + blue);

#define SET_BACKGROUND_COLOR(color) \
	printf("%s%dm",CODE_BACKGROUND_COLOR, color);

#define SET_BOLD_FONT(color) \
	printf(CODE_BOLD_FONT);

#define SET_UNDERLINE_FONT() \
	printf(CODE_UNDERLINE_FONT);

#define SET_REVERSED_FONT() \
	printf(CODE_REVERSED_FONT);

#define RESET_COLOR() \
	printf(CODE_RESET_COLOR);

#define SAUT_DE_LIGNE() \
	printf("\n");

#define CURSOR_UP(Nb) \
	printf("%s%d%s",CODE_ESCAPE_BASE,Nb,"A");

#define CURSOR_DOWN(Nb) \
	printf("%s%d%s",CODE_ESCAPE_BASE,Nb,"B");

#define CURSOR_RIGHT(Nb) \
	printf("%s%d%s",CODE_ESCAPE_BASE,Nb,"C");

#define CURSOR_LEFT(Nb) \
	printf("%s%d%s",CODE_ESCAPE_BASE,Nb,"D");

#define SET_CURSOR_COLUMN(Nb) \
	printf("%s%d%s",CODE_ESCAPE_BASE,Nb,"G");


#define SET_CURSOR_POSITION(n,m) \
	printf("%s%d;%d%s",CODE_ESCAPE_BASE,n,m,"H");

#define SAVE_CURSOR_POSITION() \
	printf("\x1B[s");

#define RESTORE_CURSOR_POSITION() \
	printf("\x1B[u");

#define CLEAR_SCREEN(Nb) \
	printf("%s%d%s",CODE_ESCAPE_BASE,Nb,"J"); //NB 1,1 ou 2

#define CLEAR_LINE(Nb) \
	printf("%s%d%s",CODE_ESCAPE_BASE,Nb,"K"); //NB 1,1 ou 2

/*----------------------------------------------------------------------------*
 * declaration des prototypes                                                 *
 *----------------------------------------------------------------------------*/

/*
 * entrée  : sans
 * sortie : sans
 * description : code de démonstration de la palette de couleur du terminal utilisant les macro
 * définies plus haut
 */
void test_colors(void);

#endif
