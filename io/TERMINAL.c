/*
 * TERMINAL.c
 *
 *  Created on: 28 mai 2020
 *      Author: jdm
 */

#include "TERMINAL.h"



void test_colors(){
	int color;
	int green, red, blue;

	/* 0 - 15 */
	  puts("System colors:");
	  for(color = 0; color<8; color++)
	  {
		  SET_BACKGROUND_COLOR(color)
		  printf(" ");
	  }
	  RESET_COLOR()
	  SAUT_DE_LIGNE()
	  for(color = 8; color<16; color++) {
		  SET_BACKGROUND_COLOR(color);
		  printf(" ");
	  }
	  RESET_COLOR()
	  SAUT_DE_LIGNE()
	  // 16 - 231
	  puts("Color cube, 6x6x6:");
	  for(red = 0; red<6; red++) {
		  for(green = 0; green<6; green++) {
			  for(blue = 0; blue<6; blue++) {
				  color = 16 + (red * 36) + (green * 6) + blue;
				  SET_BACKGROUND_COLOR(color);
				  SET_FONT_COLOR((color+6)%256)
				  printf(" %3d", color);
			  }
			  RESET_COLOR()
			  SAUT_DE_LIGNE()
		  }
	  // 232 - 255
	  puts("Grayscale ramp:");
	  for (color = 232; color < 256; color++) {
		  SET_BACKGROUND_COLOR(color);
		  printf(" ");
	  }
	  RESET_COLOR()
	  SAUT_DE_LIGNE()
	  }
}



