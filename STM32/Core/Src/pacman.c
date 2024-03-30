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

/* Enums ---------------------------------------------------------------------*/

typedef enum DIRECTION
{
	STOP,
	UP,
	DOWN,
	LEFT,
	RIGHT
} E_DIRECTION;

typedef enum OBJECT
{
	NONE,
	PAC_DOT,
	WALL,
	CHERRY
} MAZE_OBJECT;

/* Struct --------------------------------------------------------------------*/
typedef struct CELL
{
	MAZE_OBJECT object;
} S_CELL;

typedef struct MAZE
{
	S_CELL cells[MAZE_ROW_N][MAZE_COLUMN_N];
} S_MAZE;

typedef struct GHOST
{
	uint8_t i, j;
	uint8_t i_pre, j_pre;
	E_DIRECTION direction;
} S_GHOST;

typedef struct PACMAN
{
	uint8_t i, j;
	uint8_t i_pre, j_pre;
	E_DIRECTION direction;
	int score;
} S_PACMAN;

/* Private Objects -----------------------------------------------------------*/
// Pac-Man object
S_PACMAN pacman;
void pacman_draw(uint8_t i, uint8_t j, uint16_t color);
void pacman_direction_process(void);
void pacman_moving_process(void);

// Ghost object
S_GHOST ghost;
void ghost_draw(uint8_t i, uint8_t j, uint16_t color);
void ghost_direction_process(void);
void ghost_moving_process(void);

// Maze object
S_MAZE maze;
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
int TOTAL_SCORE = MAZE_ROW_N * MAZE_COLUMN_N - 1;

void game_init(void)
{
	lcd_clear(BACKGROUND_COLOR);
	lcd_draw_rectangle(MAZE_TOP_BORDER, MAZE_LEFT_BORDER, MAZE_BOTTOM_BORDER, MAZE_RIGHT_BORDER, BLACK);
	lcd_show_string(20, 250, "Extremely simple PAC-MAN", BLACK, BACKGROUND_COLOR, 16, 0);
	lcd_show_string(20, 270, "Score: ", BLACK, BACKGROUND_COLOR, 16, 0);
	lcd_show_int_num(80, 270, 0, 1, RED, BACKGROUND_COLOR, 16);

	led_7seg_set_colon(0);
	led_7seg_set_digit(0, 0, 0);
	led_7seg_set_digit(0, 1, 0);
	led_7seg_set_digit(0, 2, 0);
	led_7seg_set_digit(0, 3, 0);

	for (int i = 0; i < MAZE_ROW_N; i++)
	{
		for (int j = 0; j < MAZE_COLUMN_N; j++)
		{
			maze.cells[i][j].object = PAC_DOT;	// Assume all cells initially have a pac-dot
			pac_dot_draw(i, j, PAC_DOTS_COLOR); // Draw pac-dot on the maze
		}
	}

	for (int i = 0; i < MAZE_ROW_N; i++)
		{
			for (int j = 0; j < MAZE_COLUMN_N; j++)
			{
				if(i == 5 || i == 9)
				{
					if(j >= 4 && j <= MAZE_COLUMN_N - 4 && j != 7 && j != 8)
					{
						maze.cells[i][j].object = WALL;
						wall_draw(i, j, WALL_COLOR);
					}
				}
				if(i == 6 || i == 7 || i == 8)
				{
					if((j == 4 || j == MAZE_COLUMN_N - 4))
					{
						maze.cells[i][j].object = WALL;
						wall_draw(i, j, WALL_COLOR);
					}
				}
			}
		}

	pacman.i = PACMAN_STARTING_I;
	pacman.j = PACMAN_STARTING_J;
	pacman.i_pre = pacman.i;
	pacman.j_pre = pacman.j;
	pacman.direction = STOP;
	pacman.score = 0;
	maze.cells[pacman.i][pacman.j].object = NONE; // reset maze cell at pacman position
	pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);

	ghost.i = GHOST_STARTING_I;
	ghost.j = GHOST_STARTING_J;
	ghost.i_pre = ghost.i;
	ghost.j_pre = ghost.j;
	ghost.direction = STOP;
	ghost_draw(ghost.i, ghost.j, GHOST_COLOR);
}

/**
 * @brief  	Process game
 * @param  	None
 * @note  	Call in loop (main) every 50ms
 * @retval 	None
 */
void game_process(void)
{
	static uint8_t counter_game = 0;
	counter_game = (counter_game + 1) % 20;

	pacman_direction_process(); // Put this function here to read buttons.
	if ((button_count[15] + 1) % 60 == 0)
	{
		game_init();
		return;
	}

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

	if ((counter_game % 5) == 0)
	{ // update every 250ms
		pacman_moving_process();
		ghost_direction_process();
		ghost_moving_process();

		game_handler();
		game_draw();
	}
}

/* Private Functions ---------------------------------------------------------*/
void game_draw(void)
{
	if (pacman.i != pacman.i_pre || pacman.j != pacman.j_pre)
	{
		pacman_draw(pacman.i_pre, pacman.j_pre, BACKGROUND_COLOR);
		pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);
	}

	if (ghost.i != ghost.i_pre || ghost.j != ghost.j_pre)
	{
		ghost_draw(ghost.i_pre, ghost.j_pre, BACKGROUND_COLOR);
		if(maze.cells[ghost.i_pre][ghost.j_pre].object)
		{
			pac_dot_draw(ghost.i_pre, ghost.j_pre, PAC_DOTS_COLOR);
		}
		ghost_draw(ghost.i, ghost.j, GHOST_COLOR);
	}
}

uint8_t is_loss_condition_met()
{
	return (ghost.i == pacman.i && ghost.j == pacman.j);
}
uint8_t is_win_condition_met()
{
	return (pacman.score >= TOTAL_SCORE);
}
uint8_t has_pacman_eaten_dot()
{
	return maze.cells[pacman.i][pacman.j].object == PAC_DOT;
}
/**
 * handle game event
 */
void game_handler(void)
{
	if (is_loss_condition_met())
	{
		game_init();
		return;
	}
	if (is_win_condition_met())
	{
		game_init();
		return;
	}
	if (has_pacman_eaten_dot())
	{
		maze.cells[pacman.i][pacman.j].object = NONE;
		pacman.score += POINTS_PER_DOT;

		led_7seg_set_digit(pacman.score / 1000, 0, 0);
		led_7seg_set_digit((pacman.score / 100) % 10, 1, 0);
		led_7seg_set_digit(pacman.score / 10, 2, 0);
		led_7seg_set_digit(pacman.score % 10, 3, 0);

		lcd_show_int_num(80, 270, pacman.score, 2, RED, BACKGROUND_COLOR, 16);
	}
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
}

void pacman_moving_process(void)
{
	if (pacman.direction == UP && pacman.i > 0 && maze.cells[pacman.i - 1][pacman.j].object != WALL)
	{
		pacman.i_pre = pacman.i;
		pacman.j_pre = pacman.j;
		pacman.i--;
	}
	else if (pacman.direction == DOWN && pacman.i < MAZE_ROW_N - 1 && maze.cells[pacman.i + 1][pacman.j].object != WALL)
	{
		pacman.i_pre = pacman.i;
		pacman.j_pre = pacman.j;
		pacman.i++;
	}
	else if (pacman.direction == LEFT && pacman.j > 0 && maze.cells[pacman.i][pacman.j - 1].object != WALL)
	{
		pacman.i_pre = pacman.i;
		pacman.j_pre = pacman.j;
		pacman.j--;
	}
	else if (pacman.direction == RIGHT && pacman.j < MAZE_COLUMN_N - 1 && maze.cells[pacman.i][pacman.j + 1].object != WALL)
	{
		pacman.i_pre = pacman.i;
		pacman.j_pre = pacman.j;
		pacman.j++;
	}
	else;
}

void ghost_direction_process(void)
{
	int random_direction = rand() % 4;

	switch (random_direction)
	{
	case 0:
		ghost.direction = UP;
		break;
	case 1:
		ghost.direction = DOWN;
		break;
	case 2:
		ghost.direction = LEFT;
		break;
	case 3:
		ghost.direction = RIGHT;
		break;
	default:
		break;
	}
}

void ghost_moving_process(void)
{
	if (ghost.direction == UP && ghost.i > 0 && maze.cells[ghost.i - 1][ghost.j].object != WALL)
	{
		ghost.i_pre = ghost.i;
		ghost.j_pre = ghost.j;
		ghost.i--;
	}
	else if (ghost.direction == DOWN && ghost.i < MAZE_ROW_N - 1 && maze.cells[ghost.i + 1][ghost.j].object != WALL)
	{
		ghost.i_pre = ghost.i;
		ghost.j_pre = ghost.j;
		ghost.i++;
	}
	else if (ghost.direction == LEFT && ghost.j > 0 && maze.cells[ghost.i][ghost.j - 1].object != WALL)
	{
		ghost.i_pre = ghost.i;
		ghost.j_pre = ghost.j;
		ghost.j--;
	}
	else if (ghost.direction == RIGHT && ghost.j < MAZE_COLUMN_N - 1 && maze.cells[ghost.i][ghost.j + 1].object != WALL)
	{
		ghost.i_pre = ghost.i;
		ghost.j_pre = ghost.j;
		ghost.j++;
	}
	else
		;
}

void pac_dot_draw(uint8_t i, uint8_t j, uint16_t color)
{
	lcd_draw_circle(((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j + ((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) / 2,
					((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) * i + ((MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / MAZE_ROW_N) / 2, color, 3, 1);
}

void wall_draw(uint8_t i, uint8_t j, uint16_t color)
{
	lcd_fill(((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * j,
			((MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER) / MAZE_COLUMN_N) * i,
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
