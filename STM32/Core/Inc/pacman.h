/*
 * pacman.h
 */

#ifndef INC_PACMAN_H_
#define INC_PACMAN_H_

/* Includes */
#include <stdint.h>

/* Constants */
#define MAZE_COLUMN_N      		15
#define MAZE_ROW_N         		15
#define MAZE_CELL_WIDTH    		20

#define MAZE_TOP_BORDER			0
#define MAZE_BOTTOM_BORDER 		240
#define MAZE_LEFT_BORDER		0
#define MAZE_RIGHT_BORDER		240

#define BACKGROUND_COLOR		WHITE
#define PACMAN_COLOR			YELLOW
#define PAC_DOTS_COLOR			BROWN
#define WALL_COLOR				DARKBLUE

#define GHOST_0_COLOR			RED
#define GHOST_1_COLOR			BLACK
#define GHOST_2_COLOR			BRRED
#define GHOST_3_COLOR			LGRAYBLUE

#define PACMAN_STARTING_I		MAZE_COLUMN_N / 2
#define PACMAN_STARTING_J		MAZE_ROW_N / 2

#define GHOST_0_STARTING_I		1
#define GHOST_0_STARTING_J		1

#define GHOST_1_STARTING_I		1
#define GHOST_1_STARTING_J		MAZE_COLUMN_N - 2

#define GHOST_2_STARTING_I		MAZE_ROW_N - 2
#define GHOST_2_STARTING_J		1

#define GHOST_3_STARTING_I		MAZE_ROW_N - 2
#define GHOST_3_STARTING_J		MAZE_COLUMN_N - 2

/* Functions */
void game_init(void);
void game_process(void);

#endif /* INC_BUTTON_H_ */
