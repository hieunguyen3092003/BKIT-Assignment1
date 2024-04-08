/*
 * pacman.c
 */

/* Includes ------------------------------------------------------------------*/
#include "pacman.h"
#include "button.h"
#include "lcd.h"
#include "led_7seg.h"
#include "gpio.h"

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

/* Enums ---------------------------------------------------------------------*/

typedef enum DIRECTION
{
	STOP,
	UP,
	DOWN,
	LEFT,
	RIGHT
} DIRECTION;

typedef enum OBJECT
{
	NONE,
	PAC_DOT,
	WALL,
	CHERRY
} OBJECT;

/* Struct --------------------------------------------------------------------*/

typedef struct MAZE
{
	OBJECT cells[MAZE_ROW_N][MAZE_COLUMN_N];
} MAZE;

typedef struct GHOST
{
	uint8_t i, j;
	uint8_t i_pre, j_pre;
	DIRECTION direction;
} GHOST;

typedef struct PACMAN
{
	uint8_t i, j;
	uint8_t i_pre, j_pre;
	DIRECTION direction;
	int score;
} PACMAN;

/* Private Objects -----------------------------------------------------------*/
// Pac-Man object
PACMAN pacman;
void pacman_draw(uint8_t i, uint8_t j, uint16_t color);
void pacman_direction_process(void);
void pacman_moving_process(void);

// Ghost object
GHOST ghost_0;
GHOST ghost_1;
GHOST ghost_2;
GHOST ghost_3;
void ghost_draw(uint8_t i, uint8_t j, uint16_t color);
void ghost_direction_process(void);
void ghost_moving_process(void);

// Maze object
MAZE maze;

void pac_dot_draw(uint8_t i, uint8_t j, uint16_t color);
void wall_draw(uint8_t i, uint8_t j, uint16_t color);

// Game Engine object
void game_draw(void);
void game_handler(void);

/* Declare Private Support Functions -----------------------------------------*/
uint8_t isButtonUp(void);
uint8_t isButtonDown(void);
uint8_t isButtonLeft(void);
uint8_t isButtonRight(void);

/* Public Functions ----------------------------------------------------------*/
/**
 * @brief  	Init Pac-Man game
 * @param  	None
 * @note  	Call when you want to init game
 * @retval 	None
 */

const MAZE DEFAULT_MAZE = {
	{
		{WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL},
		{WALL, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, WALL},
		{WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL},
		{WALL, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, WALL},
		{WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL},
		{WALL, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, WALL},
		{WALL, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, WALL, WALL, WALL, WALL, PAC_DOT, WALL, WALL},
		{NONE, NONE, NONE, NONE, PAC_DOT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, PAC_DOT, NONE, NONE},
		{WALL, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, WALL, WALL, WALL, WALL, PAC_DOT, WALL, WALL},
		{WALL, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, WALL},
		{WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL},
		{WALL, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, WALL},
		{WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL, WALL, WALL, PAC_DOT, WALL},
		{WALL, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, PAC_DOT, WALL},
		{WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL, WALL}
	}
};
const int TOTAL_SCORE = 100; // if re-assign the map remember to change the TOTAL_SCORE
const int POINTS_PER_DOT = 1;
bool game_loop = true;
void game_init(void)
{
	game_loop = true;
	maze = DEFAULT_MAZE;

	lcd_clear(BACKGROUND_COLOR); // set bg color to white
	lcd_draw_rectangle(MAZE_TOP_BORDER, MAZE_LEFT_BORDER, MAZE_BOTTOM_BORDER, MAZE_RIGHT_BORDER, BLACK);

	lcd_show_string(20, 250, "Extremely simple PAC-MAN", BLACK, BACKGROUND_COLOR, 16, 0);
	lcd_show_string(20, 270, "Score: ", BLACK, BACKGROUND_COLOR, 16, 0);
	lcd_show_int_num(80, 270, 0, 1, RED, BACKGROUND_COLOR, 16);

//	led 7 seg
	led_7seg_set_colon(0);
	led_7seg_set_digit(0, 0, 0);
	led_7seg_set_digit(0, 1, 0);
	led_7seg_set_digit(0, 2, 0);
	led_7seg_set_digit(0, 3, 0);

	//	DRAW MAP
	for (int i = 0; i < MAZE_ROW_N; i++)
	{
		for (int j = 0; j < MAZE_COLUMN_N; j++)
		{
			if(maze.cells[i][j] == PAC_DOT)
			{
				pac_dot_draw(i, j, PAC_DOTS_COLOR); // Draw pac-dot on the maze
			}
			else if(maze.cells[i][j] == WALL)
			{
				wall_draw(i, j, WALL_COLOR);
			}
			else if(maze.cells[i][j] == CHERRY)
			{
//				cherry_draw(i, j, CHERRY_COLOR);
			}
			else;
		}
	}

//	at first you must check PACMAN_STARTING_I PACMAN_STARTING_J is not a wall
	maze.cells[PACMAN_STARTING_I][PACMAN_STARTING_J] = NONE; // reset maze cell at pacman position
	pacman.i = PACMAN_STARTING_I;
	pacman.j = PACMAN_STARTING_J;
	pacman.i_pre = pacman.i;
	pacman.j_pre = pacman.j;
	pacman.direction = STOP;
	pacman.score = 0;
	pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);

//	at first you must check GHOST_STARTING_I GHOST_STARTING_J is not a wall
	if(maze.cells[GHOST_0_STARTING_I][GHOST_0_STARTING_J] == WALL)
	{
		maze.cells[GHOST_0_STARTING_I][GHOST_0_STARTING_J] = NONE;
	}
	ghost_0.i = GHOST_0_STARTING_I;
	ghost_0.j = GHOST_0_STARTING_J;
	ghost_0.i_pre = ghost_0.i;
	ghost_0.j_pre = ghost_0.j;
	ghost_0.direction = STOP;
	ghost_draw(ghost_0.i, ghost_0.j, GHOST_0_COLOR);

	if(maze.cells[GHOST_1_STARTING_I][GHOST_1_STARTING_J] == WALL)
	{
		maze.cells[GHOST_1_STARTING_I][GHOST_1_STARTING_J] = NONE;
	}
	ghost_1.i = GHOST_1_STARTING_I;
	ghost_1.j = GHOST_1_STARTING_J;
	ghost_1.i_pre = ghost_1.i;
	ghost_1.j_pre = ghost_1.j;
	ghost_1.direction = STOP;
	ghost_draw(ghost_1.i, ghost_1.j, GHOST_1_COLOR);

	if(maze.cells[GHOST_2_STARTING_I][GHOST_2_STARTING_J] == WALL)
	{
		maze.cells[GHOST_2_STARTING_I][GHOST_2_STARTING_J] = NONE;
	}
	ghost_2.i = GHOST_2_STARTING_I;
	ghost_2.j = GHOST_2_STARTING_J;
	ghost_2.i_pre = ghost_2.i;
	ghost_2.j_pre = ghost_2.j;
	ghost_2.direction = STOP;
	ghost_draw(ghost_2.i, ghost_2.j, GHOST_2_COLOR);

	if(maze.cells[GHOST_3_STARTING_I][GHOST_3_STARTING_J] == WALL)
	{
		maze.cells[GHOST_3_STARTING_I][GHOST_3_STARTING_J] = NONE;
	}
	ghost_3.i = GHOST_3_STARTING_I;
	ghost_3.j = GHOST_3_STARTING_J;
	ghost_3.i_pre = ghost_3.i;
	ghost_3.j_pre = ghost_3.j;
	ghost_3.direction = STOP;
	ghost_draw(ghost_3.i, ghost_3.j, GHOST_3_COLOR);
}

/**
 * @brief  	Process game
 * @param  	None
 * @note  	Call in loop (main) every 50ms
 * @retval 	None
 */
void game_process(void)
{
	if(game_loop)
	{
		static uint8_t counter_game = 0;
		counter_game = (counter_game + 1) % 20;

		pacman_direction_process(); // Put this function here to read buttons.
		if ((button_count[15] + 1) % 60 == 0)
		{
			game_init();
			return;
		}

		if ((counter_game % 5) == 0)
		{ // update every 250ms
			pacman_moving_process();
			ghost_direction_process();
			ghost_moving_process();

			game_draw();

			game_handler();
		}

	//	blink led according to points
		if (pacman.score < TOTAL_SCORE * 30 / 100)
		{
			if (counter_game == 0)
			{
				HAL_GPIO_TogglePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin);
				HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, RESET);
				HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, RESET);
			}
		}
		else if (pacman.score >= TOTAL_SCORE * 75 / 100)
		{
			if ((counter_game % 5) == 0)
			{
				HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
				HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, RESET);
				HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, RESET);
			}
		}
		else
		{
			if ((counter_game % 10) == 0)
			{
				HAL_GPIO_TogglePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin);
				HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, RESET);
				HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, RESET);
			}
		}
	}
	else
	{
		static uint8_t counter_game = 0;
		counter_game = (counter_game + 1) % 60;

		if(counter_game == 0)
		{
			game_init();
			return;
		}
	}
}

/* Private Functions ---------------------------------------------------------*/
void game_draw(void)
{
	if (pacman.direction != STOP  || pacman.direction != STOP)
	{
		pacman_draw(pacman.i_pre, pacman.j_pre, BACKGROUND_COLOR);
		pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);
	}

	ghost_draw(ghost_0.i_pre, ghost_0.j_pre, BACKGROUND_COLOR);
	if(maze.cells[ghost_0.i_pre][ghost_0.j_pre] != NONE)
	{
		pac_dot_draw(ghost_0.i_pre, ghost_0.j_pre, PAC_DOTS_COLOR);
	}
	ghost_draw(ghost_0.i, ghost_0.j, GHOST_0_COLOR);

	ghost_draw(ghost_1.i_pre, ghost_1.j_pre, BACKGROUND_COLOR);
	if(maze.cells[ghost_1.i_pre][ghost_1.j_pre] == PAC_DOT)
	{
		pac_dot_draw(ghost_1.i_pre, ghost_1.j_pre, PAC_DOTS_COLOR);
	}
	ghost_draw(ghost_1.i, ghost_1.j, GHOST_1_COLOR);

	ghost_draw(ghost_2.i_pre, ghost_2.j_pre, BACKGROUND_COLOR);
	if(maze.cells[ghost_2.i_pre][ghost_2.j_pre] != NONE)
	{
		pac_dot_draw(ghost_2.i_pre, ghost_2.j_pre, PAC_DOTS_COLOR);
	}
	ghost_draw(ghost_2.i, ghost_2.j, GHOST_2_COLOR);

	ghost_draw(ghost_3.i_pre, ghost_3.j_pre, BACKGROUND_COLOR);
	if(maze.cells[ghost_3.i_pre][ghost_3.j_pre] != NONE)
	{
		pac_dot_draw(ghost_3.i_pre, ghost_3.j_pre, PAC_DOTS_COLOR);
	}
	ghost_draw(ghost_3.i, ghost_3.j, GHOST_3_COLOR);
}

/**
 * handle game event
 */
uint8_t is_loss_condition_met()
{
	return ((ghost_0.i == pacman.i && ghost_0.j == pacman.j) ||
			(ghost_1.i == pacman.i && ghost_1.j == pacman.j) ||
			(ghost_2.i == pacman.i && ghost_2.j == pacman.j) ||
			(ghost_3.i == pacman.i && ghost_3.j == pacman.j) ||
			(ghost_0.i_pre == pacman.i_pre && ghost_0.j_pre == pacman.j_pre) ||
			(ghost_1.i_pre == pacman.i_pre && ghost_1.j_pre == pacman.j_pre) ||
			(ghost_2.i_pre == pacman.i_pre && ghost_2.j_pre == pacman.j_pre) ||
			(ghost_3.i_pre == pacman.i_pre && ghost_3.j_pre == pacman.j_pre)   );
}
uint8_t is_win_condition_met()
{
	return (pacman.score >= TOTAL_SCORE);
}
uint8_t has_pacman_eaten_dot()
{
	return maze.cells[pacman.i][pacman.j] == PAC_DOT;
}
void game_handler(void)
{
	if (is_loss_condition_met())
	{
		lcd_clear(BACKGROUND_COLOR); // set bg color to white
		lcd_show_string(80, 100, "YOU LOSE", BLACK, BACKGROUND_COLOR, 16, 0);

		game_loop = false;
	}
	else if (is_win_condition_met())
	{
		lcd_clear(BACKGROUND_COLOR); // set bg color to white
		lcd_show_string(80, 100, "YOU WIN", BLACK, BACKGROUND_COLOR, 16, 0);

		game_loop = false;
	}
	else if (has_pacman_eaten_dot())
	{
		maze.cells[pacman.i][pacman.j] = NONE;
		pacman.score += POINTS_PER_DOT;

		led_7seg_set_digit(pacman.score / 1000, 0, 0);
		led_7seg_set_digit((pacman.score / 100) % 10, 1, 0);
		led_7seg_set_digit(pacman.score / 10, 2, 0);
		led_7seg_set_digit(pacman.score % 10, 3, 0);

		lcd_show_int_num(80, 270, pacman.score, 2, RED, BACKGROUND_COLOR, 16);
	}
	else;
}

void pacman_direction_process(void)
{
	if (isButtonUp())
	{
		pacman.direction = UP;
	}
	else if (isButtonDown())
	{
		pacman.direction = DOWN;
	}
	else if (isButtonLeft())
	{
		pacman.direction = LEFT;
	}
	else if (isButtonRight())
	{
		pacman.direction = RIGHT;
	}
	else;
}

void pacman_moving_process(void)
{
	if (pacman.direction == UP && pacman.i > 0 && maze.cells[pacman.i - 1][pacman.j] != WALL)
	{
		pacman.i_pre = pacman.i;
		pacman.j_pre = pacman.j;
		pacman.i--;
	}
	else if (pacman.direction == DOWN && pacman.i < MAZE_ROW_N - 1 && maze.cells[pacman.i + 1][pacman.j] != WALL)
	{
		pacman.i_pre = pacman.i;
		pacman.j_pre = pacman.j;
		pacman.i++;
	}
	else if (pacman.direction == LEFT && pacman.j > 0 && maze.cells[pacman.i][pacman.j - 1] != WALL)
	{
		pacman.i_pre = pacman.i;
		pacman.j_pre = pacman.j;
		pacman.j--;
	}
	else if (pacman.direction == RIGHT && pacman.j < MAZE_COLUMN_N - 1 && maze.cells[pacman.i][pacman.j + 1] != WALL)
	{
		pacman.i_pre = pacman.i;
		pacman.j_pre = pacman.j;
		pacman.j++;
	}
	else
	{
		pacman.direction = STOP;
	}
}

void ghost_direction_process(void)
{
	int random_direction = rand() % 4;

	switch (random_direction)
	{
	case 0:
		ghost_0.direction = UP;
		break;
	case 1:
		ghost_0.direction = DOWN;
		break;
	case 2:
		ghost_0.direction = LEFT;
		break;
	case 3:
		ghost_0.direction = RIGHT;
		break;
	default:
		break;
	}

	switch (random_direction)
	{
	case 0:
		ghost_1.direction = DOWN;
		break;
	case 1:
		ghost_1.direction = UP;
		break;
	case 2:
		ghost_1.direction = RIGHT;
		break;
	case 3:
		ghost_1.direction = LEFT;
		break;
	default:
		break;
	}

	switch (random_direction)
	{
	case 0:
		ghost_2.direction = LEFT;
		break;
	case 1:
		ghost_2.direction = RIGHT;
		break;
	case 2:
		ghost_2.direction = UP;
		break;
	case 3:
		ghost_2.direction = DOWN;
		break;
	default:
		break;
	}

	switch (random_direction)
	{
	case 0:
		ghost_3.direction = RIGHT;
		break;
	case 1:
		ghost_3.direction = LEFT;
		break;
	case 2:
		ghost_3.direction = DOWN;
		break;
	case 3:
		ghost_3.direction = UP;
		break;
	default:
		break;
	}
}

void ghost_moving_process(void)
{
	if (ghost_0.direction == UP && ghost_0.i > 0 && maze.cells[ghost_0.i - 1][ghost_0.j] != WALL)
	{
		ghost_0.i_pre = ghost_0.i;
		ghost_0.j_pre = ghost_0.j;
		ghost_0.i--;
	}
	else if (ghost_0.direction == DOWN && ghost_0.i < MAZE_ROW_N - 1 && maze.cells[ghost_0.i + 1][ghost_0.j] != WALL)
	{
		ghost_0.i_pre = ghost_0.i;
		ghost_0.j_pre = ghost_0.j;
		ghost_0.i++;
	}
	else if (ghost_0.direction == LEFT && ghost_0.j > 0 && maze.cells[ghost_0.i][ghost_0.j - 1] != WALL)
	{
		ghost_0.i_pre = ghost_0.i;
		ghost_0.j_pre = ghost_0.j;
		ghost_0.j--;
	}
	else if (ghost_0.direction == RIGHT && ghost_0.j < MAZE_COLUMN_N - 1 && maze.cells[ghost_0.i][ghost_0.j + 1] != WALL)
	{
		ghost_0.i_pre = ghost_0.i;
		ghost_0.j_pre = ghost_0.j;
		ghost_0.j++;
	}
	else;

	if (ghost_1.direction == UP && ghost_1.i > 0 && maze.cells[ghost_1.i - 1][ghost_1.j] != WALL)
	{
		ghost_1.i_pre = ghost_1.i;
		ghost_1.j_pre = ghost_1.j;
		ghost_1.i--;
	}
	else if (ghost_1.direction == DOWN && ghost_1.i < MAZE_ROW_N - 1 && maze.cells[ghost_1.i + 1][ghost_1.j] != WALL)
	{
		ghost_1.i_pre = ghost_1.i;
		ghost_1.j_pre = ghost_1.j;
		ghost_1.i++;
	}
	else if (ghost_1.direction == LEFT && ghost_1.j > 0 && maze.cells[ghost_1.i][ghost_1.j - 1] != WALL)
	{
		ghost_1.i_pre = ghost_1.i;
		ghost_1.j_pre = ghost_1.j;
		ghost_1.j--;
	}
	else if (ghost_1.direction == RIGHT && ghost_1.j < MAZE_COLUMN_N - 1 && maze.cells[ghost_1.i][ghost_1.j + 1] != WALL)
	{
		ghost_1.i_pre = ghost_1.i;
		ghost_1.j_pre = ghost_1.j;
		ghost_1.j++;
	}
	else;

	if (ghost_2.direction == UP && ghost_2.i > 0 && maze.cells[ghost_2.i - 1][ghost_2.j] != WALL)
	{
		ghost_2.i_pre = ghost_2.i;
		ghost_2.j_pre = ghost_2.j;
		ghost_2.i--;
	}
	else if (ghost_2.direction == DOWN && ghost_2.i < MAZE_ROW_N - 1 && maze.cells[ghost_2.i + 1][ghost_2.j] != WALL)
	{
		ghost_2.i_pre = ghost_2.i;
		ghost_2.j_pre = ghost_2.j;
		ghost_2.i++;
	}
	else if (ghost_2.direction == LEFT && ghost_2.j > 0 && maze.cells[ghost_2.i][ghost_2.j - 1] != WALL)
	{
		ghost_2.i_pre = ghost_2.i;
		ghost_2.j_pre = ghost_2.j;
		ghost_2.j--;
	}
	else if (ghost_2.direction == RIGHT && ghost_2.j < MAZE_COLUMN_N - 1 && maze.cells[ghost_2.i][ghost_2.j + 1] != WALL)
	{
		ghost_2.i_pre = ghost_2.i;
		ghost_2.j_pre = ghost_2.j;
		ghost_2.j++;
	}
	else;

	if (ghost_3.direction == UP && ghost_3.i > 0 && maze.cells[ghost_3.i - 1][ghost_3.j] != WALL)
	{
		ghost_3.i_pre = ghost_3.i;
		ghost_3.j_pre = ghost_3.j;
		ghost_3.i--;
	}
	else if (ghost_3.direction == DOWN && ghost_3.i < MAZE_ROW_N - 1 && maze.cells[ghost_3.i + 1][ghost_3.j] != WALL)
	{
		ghost_3.i_pre = ghost_3.i;
		ghost_3.j_pre = ghost_3.j;
		ghost_3.i++;
	}
	else if (ghost_3.direction == LEFT && ghost_3.j > 0 && maze.cells[ghost_3.i][ghost_3.j - 1] != WALL)
	{
		ghost_3.i_pre = ghost_3.i;
		ghost_3.j_pre = ghost_3.j;
		ghost_3.j--;
	}
	else if (ghost_3.direction == RIGHT && ghost_3.j < MAZE_COLUMN_N - 1 && maze.cells[ghost_3.i][ghost_3.j + 1] != WALL)
	{
		ghost_3.i_pre = ghost_3.i;
		ghost_3.j_pre = ghost_3.j;
		ghost_3.j++;
	}
	else;
}

void pac_dot_draw(uint8_t i, uint8_t j, uint16_t color)
{
//	lcd_draw_circle(((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j + ((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) / 2,
//					((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) * i + ((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) / 2, color, 3, 1);

	lcd_fill(((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j + ((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) / 3,
			((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_ROW_N) * i + ((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) / 3,
			((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j + ((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) - ((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) / 3,
			((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) * i + ((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) - ((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) / 3, color);
}

void wall_draw(uint8_t i, uint8_t j, uint16_t color)
{
	lcd_fill(((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j,
			((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) * i,
			((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j + ((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N),
			((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) * i + ((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N), color);
}

void pacman_draw(uint8_t i, uint8_t j, uint16_t color)
{
	lcd_draw_circle(((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j + ((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) / 2,
					((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) * i + ((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) / 2, color, 6, 1);
}

void ghost_draw(uint8_t i, uint8_t j, uint16_t color)
{
	lcd_draw_circle(((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j + ((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) / 2,
					((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) * i + ((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) / 2, color, 6, 1);
}

uint8_t isButtonUp(void)
{
	if (button_count[1] == 1)
	{
		return 1;
	}
	return 0;
}

uint8_t isButtonDown(void)
{
	if (button_count[9] == 1)
	{
		return 1;
	}
	return 0;
}

uint8_t isButtonLeft(void)
{
	if (button_count[4] == 1)
	{
		return 1;
	}
	return 0;
}

uint8_t isButtonRight(void)
{
	if (button_count[6] == 1)
	{
		return 1;
	}
	return 0;
}
