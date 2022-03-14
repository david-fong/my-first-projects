/* Author:  David Fong
 * Date:    April 26th, 2018
 * Purpose: Simulates MineSweeper in C++ !
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* Special Constants*/
#define NROWS 20 /* Would be same area and fit alphabet coordinates if 20x24 */
#define NCOLS 24
#define NBOMBS 99

#define TRUE 1
#define FALSE 0
#define IS_BOMB 1

#define MOVE_LENGTH 10
#define MACT 0
	/* Below: Used in flagRemaining calculations in playGame(). */
	#define FIRST_MOVE -2
	#define FLAG 1
	#define SWEEP 0
#define MROW 1
#define MCOL 2
#define MSBO 3 /* Move: Suppress Board Once. */

#define LOSE 0
#define WIN 1
#define RESTART 2
#define QUIT 3

/* Function prototypes */
void resetBoard(char board[][NCOLS + 4]);
int userInputInvalid(int board, char action, int row, int col, int move[]);
void printHelp(void);
int getMove(int answer[][NCOLS + 2], int hints[][NCOLS + 2], char board[][NCOLS + 4], int move[]);
int introSequence(int answer[][NCOLS + 2], int hints[][NCOLS + 2], char board[][NCOLS + 4], int highScore, int move[]);

void resetIntegerArray(int array[][NCOLS + 2]);
int randomCoordinate(int range);
void newAnswer(int answer[][NCOLS + 2], int move[]);

int sumAdjacent(int answer[][NCOLS + 2], int row, int col);
void prepareHints(int answer[][NCOLS + 2], int hints[][NCOLS + 2]);

void printBoardToScreen(char board[][NCOLS + 4]);
int showAnswer(int answer[][NCOLS + 2], char board[][NCOLS + 4]);
int playGame(int answer[][NCOLS + 2], int hints[][NCOLS + 2], char board[][NCOLS + 4], int move[]);

void saveBoardToFile(char board[][NCOLS + 4], int score, int trueBombsRemaining);

/* Debug functions. */
void printIntegerArray(int hints[][NCOLS + 2]);

int main(void) {
	int result = 0, startTime = 0, endTime = 0;
	int score = 0, highScore = 0, playAgain = 0;
	int trueBombsRemaining = 0;
	int saveBoard = 0;

	/* Below: Contains ones in the coordinates of bombs and zero elsewhere. Constant throughout each game. */
	int answer[NROWS + 2][NCOLS + 2] = { 0 };
	/* Below: Contains the number of adjacent tiles containing bombs. Constant throughout game. */
	int hints[NROWS + 2][NCOLS + 2] = { 0 };
	/* Below: Contains the 2D string visible to the player. */
	char board[NROWS + 2][NCOLS + 4] = { 0 };
	/* Below: Contains the action and coordinate of the user's choice on the current move. */
	int move[MOVE_LENGTH] = { 0 };
	
	do {
		/* Clean slate and initialize new game board. */
		playAgain = FALSE;
		srand((unsigned)time(NULL));
		resetBoard(board);
		move[MACT] = FIRST_MOVE;
		result = WIN;
		saveBoard = 0;
		startTime = introSequence(answer, hints, board, highScore, move);
		newAnswer(answer, move);
		prepareHints(answer, hints);

		/* Debugging prepareHints(). */
		printIntegerArray(answer);
		printIntegerArray(hints);

		board[move[MROW]][move[MCOL]] = hints[move[MROW]][move[MCOL]] + '0';

		/* Enter main game loop. */
		result = playGame(answer, hints, board, move);

		if (result == QUIT) {
			return 0;
		}
		if (result == RESTART) {
			playAgain = TRUE;
		}
		else {
			/* Check if the player really won. */
			trueBombsRemaining = showAnswer(answer, board);
			if (trueBombsRemaining > 0) {
				result = LOSE;
			}

			/* Game over sequence. */
			endTime = (int)time(NULL);
			score = endTime - startTime;

			if (result == LOSE) {
				printf("Onoes- we exploded D:\nThat's okay. Better luck next time :)\n");
				printf("In case you were wondering, this run lasted %d seconds.\n", score);
			}

			if (result == WIN) {
				printf("Hey, nice! You beat MineSweeper in C!\n Your score was %d seconds.\n", score);
				if (score < highScore) {
					highScore = score;
					printf("Congratulations! You set a new high score!\n");
				}
			}

			/* Prompt to save game board to a text file. */
			printf("Would you like to save your final board to a text file? (Press 0 for no or 1 for yes.)\n");
			scanf("%d", &saveBoard);
			if (saveBoard) {
				saveBoardToFile(board, score, trueBombsRemaining);
			}

			printf("\nThanks for playing!\n\nPress 0 to quit the game or press one to play a new game :)\n");
			scanf("%d", &playAgain);
		}
		printf("\n\n");
	} while (playAgain == TRUE);

	/* Unneccessary closing code. */
	system("PAUSE");
	return 0;
}

#define BLANK '.'
#define CORNER '+'
/* Purpose: Resets the character array visible to the player for a new game.
 */
void resetBoard(char board[][NCOLS + 4]) {
	int atRow = 0, atCol = 0;

	/* Fills the borders with coordinate values */
	for (atRow = 0; atRow < NROWS; atRow++) {
		board[atRow + 1][0] = board[atRow + 1][NCOLS + 1] = atRow + 'A';
	}
	for (atCol = 0; atCol < NCOLS; atCol++) {
		board[0][atCol + 1] = board[NROWS + 1][atCol + 1] = atCol + 'A';
	}

	/* Fills the body of the board with BLANK. */
	for (atRow = 0; atRow < NROWS; atRow++) {
		for (atCol = 0; atCol < NCOLS; atCol++) {
			board[atRow + 1][atCol + 1] = BLANK;
		}
	}

	/* Fills the column past the right border of the board with 'newline' characters,
	 * and the (last) column further right with NULL characters. */
	for (atRow = 0; atRow < NROWS + 2; atRow++) {
		board[atRow][NCOLS + 2] = '\n';
		board[atRow][NCOLS + 3] = '\0';
	}

	/* Fills in the corners so it prints properly. */
	board[0][0] = board[0][NCOLS + 1] = board[NROWS + 1][0] = board[NROWS + 1][NCOLS + 1] = CORNER;
}

#define FLAG_CHAR 'F'
#define FLAGGED_CHAR '*'
#define SWEEP_CHAR 'S'
#define RING_SWEEP_CHAR 'D'
#define RESTART_CHAR 'R'
#define QUIT_CHAR 'Q'
/* Purpose: Checks if the user's move input command is invalid. If so, prints an explanation and re-prompt.
 * Param:   userInput[] - The string containing the user's command from getMove().
 * Param:   move[] - Address comes from main(). Used to check whether it is the first move.
 * Return:  TRUE if the user's move is invalid, or FALSE otherwise.
 */
int userInputInvalid(int board, char action, int row, int col, int move[]) {
	/* Below: return TRUE if either: the player is trying to restart or quit on the first move,
	 * (Only allows player to restart or quit after first move since the first call to
	 * getMove() is inside introSequence(), which cannot return this type of result to main()). */
	if (move[MACT] == FIRST_MOVE && (action == RESTART_CHAR || action == QUIT_CHAR)) {
		printf("Error: You cannot resart or quit on your first move. Please try again.\n");
		return TRUE;
	}
	/* Cont. or one of the coordinates is not within the boundaries defined by the special constants, */
	if ((row - 1 < 0 || row - 1 >= NROWS) ||
		(col - 1 < 0 || col - 1 >= NCOLS)) {
		printf("Error: The coordinates you specified were not in the range of the board.\nPlease try again.\n");
		return TRUE;
	}
	/*  Cont. or the user is trying to sweep a tile other than a BLANK, */
	if (action == SWEEP_CHAR && board != BLANK) {
		printf("Error: You cannot sweep a number tile or a flagged tile. Please try again.\n");
		return TRUE;
	}
	/*  Cont. or the user is trying to ring-sweep a tile other than a number tile, */
	if (action == RING_SWEEP_CHAR && (board < '0' || board > '8')) {
		printf("Error: You cannot ring-sweep a non-number tile. Please try again.\n");
		return TRUE;
	}
	/* Cont. or the user is trying to flag a tile that is neither a BLANK nor a FLAGGED, */
	if (action == FLAG_CHAR && (board != BLANK && board != FLAGGED_CHAR)) {
		printf("Error: You cannot flag an uncovered number tile. Please try again.\n");
		return TRUE;
	}
	/* Cont. or action is none of flag, sweep, restart, or quit, */
	if (action != SWEEP_CHAR && action != RING_SWEEP_CHAR && action != FLAG_CHAR &&
		action != RESTART_CHAR && action != QUIT_CHAR) {
		printf("Error: The action was neither %c, %c, %c, %c, nor %c. Please try again\n", SWEEP_CHAR, RING_SWEEP_CHAR, FLAG_CHAR, RESTART_CHAR, QUIT_CHAR);
		return TRUE;
	}
	return FALSE;
}

#define SUPPRESS_BOARD_ONCE_CHAR 'X'
/* Purpose: Prints a message of all controls in the playGame() sequence.
 */
void printHelp(void) {
	printf("Controls:\n");
	printf("Please enter a coordinate (action, row letter, column letter) for your next move.\n");
	printf("Actions: %c will denote a flag, and %c will denote uncovering a tile (ex. %cAA).\n", FLAG_CHAR, SWEEP_CHAR, SWEEP_CHAR);
	printf("Additionally, the action %c will denote a 'ring-sweep,' which sweeps every blank tile adjacent to the coordinate specified.\n", RING_SWEEP_CHAR);
	printf("Alternatively, after the first move, enter %c to restart the game board or enter %c to exit the program.\n", RESTART_CHAR, QUIT_CHAR);
	printf("Enter %c as the fourth character in your command to suppress printing the board once.\n\n", SUPPRESS_BOARD_ONCE_CHAR);
}

#define HELP_CHAR 'H'
/* Purpose: Prompts the user for their next move until a valid move is entered.
 *          They can mark/unmark a tile as a bomb to avoid, or sweep a tile at a coordinate of choice.
 * Note:    Coordinates include the outside border!
 * Return:  returns LOSE if the player tries to sweep a bomb,
 *          RESTART if the player wants to start a new game,
 *          QUIT if the player wishes to quit the session and exit the program,
 *          and otherwise returns zero.
 * NOTE:    Passes action to playGame() for it to decide whether to increment/decrement/leave-as-is 
 *          flagsRemaining via the zeroth index of move[]. If move[MS(uppress)B(oard)O(nce)] is TRUE,
 *          playGame() will not print the board to the screen for that turn.
 */
int getMove(int answer[][NCOLS + 2], int hints[][NCOLS + 2], char board[][NCOLS + 4], int move[]) {
	int index = 0;
	char userInput[MOVE_LENGTH] = { 0 };
	int row = 0, col = 0; /* Used for each entered coordinate. */
	int atRow = 0, atCol = 0; /* Used to conduct a ring-sweep. */
	int numFlagsAdjacent = 0;
	move[MSBO] = FALSE; /* Reset MSBO. */

	printf("Your move! (Enter 'H' for help): ");
	/* Code to continuously prompt and check for a valid move. */
	do {
		scanf("%9s", userInput);

		/* Checks special case of whether the user needs help with controls, or wants to restart or quit the game. */
		if (userInput[MACT] == HELP_CHAR) {
			printHelp();
			scanf("%9s", userInput);
		}
		row = userInput[MROW] - 'A' + 1;
		col = userInput[MCOL] - 'A' + 1;
	} while (userInputInvalid(board[row][col], userInput[MACT], row, col, move));

	/* Code to translate a user move from characters to integers for safe initialization.
	 * Only used when generating the answer array and when displaying the starting move in playGame(). */
	if (move[MACT] == FIRST_MOVE) {
		move[MROW] = row;
		move[MCOL] = col;
		move[MACT] = SWEEP;
	}
	if (userInput[MSBO] == SUPPRESS_BOARD_ONCE_CHAR) {
		move[MSBO] = TRUE;
	}
	if (userInput[MACT] == RESTART_CHAR) {
		return RESTART;
	}
	else if (userInput[MACT] == QUIT_CHAR) {
		return QUIT;
	}

	/* Execute valid modification of board on following (non-first) moves. */
	else {
		/* Check basic lose condition of whether player swept/ring-swept a bomb. */
		if (userInput[MACT] == SWEEP_CHAR && answer[row][col] == IS_BOMB) {
			move[MACT] = SWEEP;
			return LOSE;
		}

		else if (userInput[MACT] == RING_SWEEP_CHAR) {
			for (atRow = -1; atRow < 2; atRow++) {
				for (atCol = -1; atCol < 2; atCol++) {
					if (board[row + atRow][col + atCol] == FLAGGED_CHAR) {
						numFlagsAdjacent++;
					}
				}
			}
			if (sumAdjacent(answer, row, col) - numFlagsAdjacent > 0) {
				move[MACT] = SWEEP;
				return LOSE;
			}
		}

		/* Reveal a safely swept hint. */
		if (userInput[MACT] == SWEEP_CHAR) {
			board[row][col] = hints[row][col] + '0';
			move[MACT] = SWEEP;
		}
		/* Reveal safely ring-swep hints.*/
		if (userInput[MACT] == RING_SWEEP_CHAR) {
			for (atRow = -1; atRow < 2; atRow++) {
				for (atCol = -1; atCol < 2; atCol++) {
					if (board[row + atRow][col + atCol] == BLANK) {
						board[row + atRow][col + atCol] = hints[row + atRow][col + atCol] + '0';
					}
				}
			}
			move[MACT] = SWEEP;
		}
		else if (userInput[MACT] == FLAG_CHAR) {
			/* Below: Flagging a blank tile. */
			if (board[row][col] == BLANK) {
				board[row][col] = FLAGGED_CHAR;
				move[MACT] = -FLAG;
			}
			/* Below: Unflagging a flagged tile. */
			else if (board[row][col] == FLAGGED_CHAR) {
				board[row][col] = BLANK;
				move[MACT] = FLAG;
			}
		}
	}

	return 1;
}

/* Purpose: Greets the player, tells them their high score in the current session,
 *          and prompts them for a starting coordinate for the new game.
 * Param:   highscore - the lowest time taken to beat the game in seconds in previous sessions.
 * Return:  The start time in seconds after the player chooses their starting coordinate
 */
int introSequence(int answer[][NCOLS + 2], int hints[][NCOLS + 2], char board[][NCOLS + 4], int highScore, int move[]) {
	int startTime = 0;
	printf("Welcome to MineSweeper in C!\n");
	printf("The current high score in this session is: %d seconds.\n", highScore);

	printBoardToScreen(board);

	printHelp();
	printf("Enter a starting coordinate for this game!\n");
	/* Below: This first move will be used to generate a valid answer array. */
	getMove(answer, hints, board, move);
	startTime = (int)time(NULL);

	printf("Good luck!\n\n");
	return startTime;
}

/* Purpose: Used to clear the answer and hints arrays before filling with new entries.
 * Param:   array[][] - The array to clear.
 */
void resetIntegerArray(int array[][NCOLS + 2]) {
	int atRow = 0, atCol = 0;

	for (atRow = 0; atRow < NROWS + 2; atRow++) {
		for (atCol = 0; atCol < NCOLS + 2; atCol++) {
			array[atRow][atCol] = 0;
		}
	}
}

/* Purpose: Generates and returns a random integer from 0 to range - 1.
 * Note:    This does not account for the outside border!
 * Return:  The random integer generated.
 */
int randomCoordinate(int range) {
	return rand() % range;
}

#define SPAWN_RADIUS 1
/* Purpose: Generates a random distribution of #NBOMBS bombs,
 *          avoiding the player's starting coordinate.
 * Param:   answer[][] - The binary array of bomb locations.
 */
void newAnswer(int answer[][NCOLS + 2], int move[]) {
	int bombsPlaced = 0;
	int row = 0, col = 0;

	/* move here has stored the user's first move from the intro sequence. */
	resetIntegerArray(answer);
	while (bombsPlaced < NBOMBS) {
		row = randomCoordinate(NROWS) + 1;
		col = randomCoordinate(NCOLS) + 1;
		/* Below: "If at least one randomly generated coordinate component is different than the user's starting coordinate
		 *        and the random coordinate does not already contain a bomb, place a bomb." */
		if (((row < move[MROW] - SPAWN_RADIUS || row > move[MROW] + SPAWN_RADIUS) ||
			 (col < move[MCOL] - SPAWN_RADIUS || col > move[MCOL] + SPAWN_RADIUS)) &&
			answer[row][col] != IS_BOMB) {
			/* Then, */
			answer[row][col] = IS_BOMB;
			bombsPlaced++;
		}
	}
}

/* Purpose: Returns the number of bombs adjacent to a specified coordinate of the game board.
 * Note:    Note: Function also used for coordinates of bombs, but this does not affect the game of win conditions of playGame().
 * Param:   answer[][] - Contains ones in bomb locations and zeros everywhere else.
 */
int sumAdjacent(int answer[][NCOLS + 2], int row, int col) {
	int atRow = -1, atCol = -1, sum = 0;
	for (atRow = -1; atRow < 2; atRow++) {
		for (atCol = -1; atCol < 2; atCol++) {
			sum += answer[row + atRow][col + atCol];
		}
	}
	return sum;
}

/* Purpose: Clears and fills a 2D array with the full board of hints:
 *          each index contains the number of bombs in the adjacent tiles (including diagonals).
 */
void prepareHints(int answer[][NCOLS + 2], int hints[][NCOLS + 2]) {
	int atRow = 0, atCol = 0;

	resetIntegerArray(hints);
	for (atRow = 0; atRow < NROWS; atRow++) {
		for (atCol = 0; atCol < NCOLS; atCol++) {
			hints[atRow + 1][atCol + 1] = sumAdjacent(answer, atRow + 1, atCol + 1);
		}
	}
}

/* Purpose: Prints the board array to the screen.
 */
void printBoardToScreen(char board[][NCOLS + 4]) {
	int row = 0;

	printf("\n");
	for (row = 0; row < NROWS + 2; row++) {
		printf("%s", board[row]);
	}
	printf("\n");
}

#define WRONGFLAG_CHAR '#'
/* Purpose: Called when player game is over to modify board array to contain all bomb locations,
            and then print the board to the screen.
 * Return:  Number of bombs missed by player.
 */
int showAnswer(int answer[][NCOLS + 2], char board[][NCOLS + 4]) {
	int trueBombsRemaining = 0;
	int atRow = 0, atCol = 0;

	/* Go through and compare answer[][] to board[][].
	   Correct and count any true bomb location not flagged,
	   and mark incorrect flags with WRONGFLAG_CHAR. */
	for (atRow = 1; atRow < NROWS + 1; atRow++) {
		for (atCol = 1; atCol < NCOLS + 1; atCol++) {
			if (answer[atRow][atCol] == IS_BOMB && board[atRow][atCol] != FLAGGED_CHAR) {
				board[atRow][atCol] = FLAGGED_CHAR;
				trueBombsRemaining++;
			}
			if (board[atRow][atCol] == FLAGGED_CHAR && answer[atRow][atCol] != IS_BOMB) {
				board[atRow][atCol] = WRONGFLAG_CHAR;
			}
		}
	}
	printBoardToScreen(board);
	return trueBombsRemaining;
}

/* Purpose: Loops through the main turn/move sequence until the player loses, wins, restarts, or quits.
 * Return:  LOSE if the player uncovers a bomb in answers, tentative WIN if the player uses all their flags,
 *          RESTART if the player wishes to quit the game and tell main() to start a new one, or
 *          QUIT tell main() to exit the program.
 */
int playGame(int answer[][NCOLS + 2], int hints[][NCOLS + 2], char board[][NCOLS + 4], int move[]) {
	int flagsRemaining = NBOMBS; /* What the player thinks is bombsRemaining. */
	int gameState = 0;

	do {
		/* Print the board and get the user's move.*/
		if (move[MSBO] != TRUE) {
			printBoardToScreen(board);
		}
		printf("Bombs apparantly remaining: %d\n", flagsRemaining);
		gameState = getMove(answer, hints, board, move);
		if (gameState == RESTART || gameState == QUIT || gameState == LOSE) {
			return gameState;
		}
		flagsRemaining += move[MACT];
	} while (flagsRemaining > 0);
	/* Below: Return a tentative WIN as the result to main for main to verify. */
	return WIN;
}

#define FILENAME_MAX_LENGTH 20
/* Purpose: Prompts the user for a string to use as a file name, opens a file with that name for writing,
 *          prints the final board to the file along with the user's score and the number of true bombs remaining,
 *          and closes the file.
 */
void saveBoardToFile(char board[][NCOLS + 4], int score, int trueBombsRemaining) {
	char fileName[FILENAME_MAX_LENGTH] = { 0 };
	int row = 0;

	printf("Please enter a name for your file no longer than %d characters: ", FILENAME_MAX_LENGTH - 1);
	scanf("%19s", fileName);

	FILE* saveFile;
	saveFile = fopen(fileName, "w");
	if (saveFile == NULL) {
		printf("Error: Could not open file for saving. Giving up on saving your file...\n\n");
	}
	else {
		for (row = 0; row < NROWS + 2; row++) {
			fprintf(saveFile, "%s", board[row]);
		}
		fprintf(saveFile, "\nScore: %d seconds\nBombs remaining: %d", score, trueBombsRemaining);
		fclose(saveFile);
		printf("Your textfile was successfully saved in this project's directory!\n\n");
	}
}

/* Debug functions. */
void printIntegerArray(int hints[][NCOLS + 2]) {
	int atRow = 0, atCol = 0;

	for (atRow = 0; atRow < NROWS + 2; atRow++) {
		for (atCol = 0; atCol < NCOLS + 2; atCol++) {
			printf("%d", hints[atRow][atCol]);
		}
		printf("\n");
	}
	printf("\n");
}