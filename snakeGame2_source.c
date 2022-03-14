/* Author: David Fong
* Date:    April 30th, 2018
* Purpose: Simulates the snake game in C++ !
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>

#define NROWS 20
#define NCOLS 25
#define MAX_LENGTH 52
#define PERIOD 200

/* Used to store and describe the board. */
#define FLOOR ' '
#define BODY 'O'
#define WALL '#'
#define APPLE '@'

/* Special constants used to describe button channels. */
#define UP_CHAR 'w'
#define LEFT_CHAR 'a'
#define DOWN_CHAR 's'
#define RIGHT_CHAR 'd'
#define QUIT_CHAR 'q'

#define UP 0
#define LEFT 1
#define DOWN 2
#define RIGHT 3
#define QUIT 4

/* Used when getting moves. */
#define NO_UPDATE -1
#define TRUE 1

/* Used to describe rows of the incrementTable[][] and history[][] arrays. */
#define ROW 0
#define COL 1

#define START_ROW 5
#define START_COL 5
#define CASUAL_MODE 0
#define HARD_MODE 1

/* Function prototypes. */
void resetHistoryAndDisplay(int history[][MAX_LENGTH], char display[][NCOLS + 4]);

int readKeys(void);
void shiftHistory(int history[][MAX_LENGTH], int length);

void spawnApple(char display[][NCOLS + 4]);

int getDifficulty(void);
void printDisplayToScreen(char display[][NCOLS + 4]);
void writeHistory(const int incrementTable[][4], int history[][MAX_LENGTH], int length, int direction);

int main(void) {
	const int incrementTable[2][4] = { { -1, 0, 1, 0 },{ 0, -1, 0, 1 } };
	int hasLost = 0, playAgain = 0, difficulty = 0;
	int readDirection = NO_UPDATE, direction = NO_UPDATE;

	char display[NROWS + 2][NCOLS + 4] = { 0 };
	int history[2][MAX_LENGTH] = { 0 };
	int headRow = 0, headCol = 0;
	int tailRow = 0, tailCol = 0;
	int length = 0, highScore = 0;
	printf("hi\n");
	do {
		/* Reset toggles and reset arrays. */
		hasLost = playAgain = length = 0;
		headRow = tailRow = START_ROW;
		headCol = tailCol = START_COL;
		resetHistoryAndDisplay(history, display);
		srand((unsigned)time(NULL));
		spawnApple(display);
		direction = NO_UPDATE;

		/* Print welcome messages, controls, and setting prompts. */
		printf("Welcome to the snake game in C!\n\n");
		printf("Control your snake with %c%c%c%c (UP, LEFT, DOWN, RIGHT).\n", UP_CHAR, LEFT_CHAR, DOWN_CHAR, RIGHT_CHAR);
		difficulty = getDifficulty();
		printf("The game will begin once you enter a starting direction...\n");
		printDisplayToScreen(display);

		/* Don't start moving until player enters first direction. */
		while (direction == NO_UPDATE) {
			Sleep(PERIOD);
			direction = readKeys();
		}

		/* Enter playing sequence. */
		do {
			/* If HARD_MODE, linearly decrease period to half its start value by the point that the length = MAX_LENGTH. */
			if (difficulty == HARD_MODE) {
				Sleep(PERIOD*(-length / 2 / MAX_LENGTH + 1));
			}
			/* Otherwise, keep the period constant. */
			else if (difficulty == CASUAL_MODE) {
				Sleep(PERIOD);
			}

			readDirection = readKeys();
			if (direction == QUIT) {
				hasLost = TRUE;
			}
			/* Update direction if user didn't enter nothing and the direction entered is not backwards. */
			else if (readDirection != NO_UPDATE && (readDirection + 2) % 4 != direction) {
				direction = readDirection;
			}
			/* Move the coordinates of the head first and then respond to what it lands on. */
			headRow += incrementTable[ROW][direction];
			headCol += incrementTable[COL][direction];
			/* Leave the move loop if the player hits a WALL or their BODY, or decides to QUIT. */
			if (display[headRow][headCol] == WALL || display[headRow][headCol] == BODY) {
				hasLost = TRUE;
			}
			else {
				if (display[headRow][headCol] == APPLE) {
					length++; /* As if the apple is the new head. */
					spawnApple(display);
				}
				else if (display[headRow][headCol] == FLOOR) {
					display[tailRow][tailCol] = FLOOR;
					/* Move the tail coordinate to the coordinate of the 2nd last body segment. */
					tailRow += history[ROW][0];
					tailCol += history[COL][0];
					/* Shift the history of moves back as if each segment will next move in the direction
					* of the segment currently ahead of it. */
					shiftHistory(history, length);
				}
				writeHistory(incrementTable, history, length, direction);
				display[headRow][headCol] = BODY;
				printf("Length = %d segments.\n", length);
			}
			printDisplayToScreen(display);
		} while (length < MAX_LENGTH - 1 && hasLost != TRUE);

		if (hasLost == TRUE) {
			printf("Onoes! The snek got rekt. Nice run, though :)\n");
		}
		else {
			printf("Wow! You beat the game!\n");
		}
		printf("Your score was %d body segments.\n", length);
		if (length > highScore) {
			printf("Congratulations! You set a new high score in this session!\n");
			highScore = length;
		}

		printf("\nThanks for playing! Press 0 to quit or press 1 to play again.\n");
		scanf("%d", &playAgain);
	} while (playAgain);

	return 0;
}

/* Purpose: Resets the history[][] and display[][] arrays.
*/
void resetHistoryAndDisplay(int history[][MAX_LENGTH], char display[][NCOLS + 4]) {
	int index = 0;
	int atRow = 0, atCol = 0;

	/* Reset History. */
	for (index = 0; index < MAX_LENGTH; index++) {
		history[ROW][index] = history[COL][index] = 0;
	}

	/*Reset display. */
	for (atRow = 0; atRow < NROWS; atRow++) {
		for (atCol = 0; atCol < NCOLS; atCol++) {
			display[atRow + 1][atCol + 1] = FLOOR;
		}
	}
	for (atCol = 0; atCol < NCOLS + 2; atCol++) {
		display[0][atCol] = display[NROWS + 1][atCol] = WALL;
	}
	for (atRow = 0; atRow < NROWS + 2; atRow++) {
		display[atRow][0] = display[atRow][NCOLS + 1] = WALL;
		display[atRow][NCOLS + 2] = '\n';
		display[atRow][NCOLS + 3] = '\0';
	}
	display[START_ROW][START_COL] = BODY;
}

/* Purpose: Goes through each button in order Up, LEFT, DOWN, RIGHT,
*          And returns the button that was ON, or otherwise returns NO_UPDATE.
*/
int readKeys(void) {
	char key = 0;

	if(_kbhit() == TRUE) {
		key = _getch();
		if (key == UP_CHAR) {
			return UP;
		}
		else if (key == LEFT_CHAR) {
			return LEFT;
		}
		else if (key == DOWN_CHAR) {
			return DOWN;
		}
		else if (key == RIGHT_CHAR) {
			return RIGHT;
		}
		else if (key == QUIT_CHAR) {
			return QUIT;
		}
	}
	

	return NO_UPDATE;
}

/* Purpose:
*/
void shiftHistory(int history[][MAX_LENGTH], int length) {
	int index = 0;

	for (index = 0; index < length; index++) {
		history[ROW][index] = history[ROW][index + 1];
		history[COL][index] = history[COL][index + 1];
	}
}

/* Purpose: Randomly spawns an apple on the FLOOR space where there isn't a BODY.
*/
void spawnApple(char display[][NCOLS + 4]) {
	int row = 0, col = 0;
	do {
		srand((unsigned)time(NULL));
		row = (int)rand() % NROWS + 1;
		col = (int)rand() % NCOLS + 1;
	} while (display[row][col] != FLOOR);
	display[row][col] = APPLE;
}

/* Purpose: Prompts the user for a difficulty mode until the user enters a valid mode.
 * Return:  The user's difficulty of choice.
 */
int getDifficulty(void) {
	int difficulty = 0;
	do {
		printf("Enter %d for hardmode where the game slowly speeds up, or 0 for casual mode: ", HARD_MODE);
		scanf("%d", &difficulty);
		if (difficulty != HARD_MODE && difficulty != CASUAL_MODE) {
			printf("Error: The number you entered was neither %d nor %d. Please try again.\n", HARD_MODE, CASUAL_MODE);
		}
	} while (difficulty != HARD_MODE && difficulty != CASUAL_MODE);
	return difficulty;
}

/* Purpose: Prints the array display[][] to the screen.
 */
void printDisplayToScreen(char display[][NCOLS + 4]) {
	int row = 0;

	printf("\n");
	for (row = 0; row < NROWS + 2; row++) {
		printf(display[row]);
	}
	printf("\n");
}

/* Purpose: Writes the direction taken by the head after reading the buttons to the current head segment's index.
 *
 */
void writeHistory(const int incrementTable[][4], int history[][MAX_LENGTH], int length, int direction) {
	history[ROW][length] = incrementTable[ROW][direction];
	history[COL][length] = incrementTable[COL][direction];
}
