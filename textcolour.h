/************************************************************************
 * Mikolaj Panka                                                        *
 * KSO 2021-z                                                           *
 * lab3                                                                 *
 ************************************************************************/
#ifndef TEXTCOLOUR_H
#define TEXTCOLOUR_H


#define RESET		0
#define BOLD 		1
#define DIM		    2
#define ITALIC 	    3
#define UNDERLINE	4
#define SLOW_BLINK  5
#define FAST_BLINK  6
#define REVERSE		7
#define HIDDEN		8

#define BG_BLACK        40
#define BLACK 		    30
#define RED		        31
#define GREEN		    32
#define YELLOW		    33
#define BLUE		    34
#define MAGENTA		    35
#define CYAN		    36
#define	WHITE		    37
#define GRAY            90
#define BRIGHT_WHITE    97

void textcolour(int attr, int fg, int bg);

#endif
