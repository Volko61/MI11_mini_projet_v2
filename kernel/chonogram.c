/*
 * chonogramme.c
 *
 *  Created on: 28 avr. 2025
 *      Author: mi11
 */

#include "chronogram.h"

#include "../io/TERMINAL.h"
#include "noyau_file_prio.h"

static int posx = 1;
static int print_hdr = 1;

void draw_tick(uint16_t task_id, char sep)
{
	uint8_t prio = task_id >> 3;

	if (posx > CHRONOGRAM_WIDTH) {
		posx = 1;
		print_hdr = 8;

	}

	for (int line = 1; line <= MAX_PRIO; ++line) {
		SET_CURSOR_POSITION(line + CHRONOGRAM_VERT_POS, posx);
		SET_FONT_COLOR(15);
		if (print_hdr) {
			SET_BACKGROUND_COLOR(0);
			printf("P%02d", line -1 );
			print_hdr--;
		}

		if (line == prio + 1) {
			SET_BACKGROUND_COLOR(task_id + 16);
			printf("%c%02d", sep, task_id);
		} else {
			SET_BACKGROUND_COLOR(0);
			printf("%c  ", sep);
		}
	}
	posx = posx + 3;
}
