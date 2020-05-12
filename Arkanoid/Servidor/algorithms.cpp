#include "algorithms.h"
#include <tchar.h>
#include <windows.h>

bool checkCoordsInLimits(int value, int min, int max) {
	return (value >= min) && (value <= max);
}

bool overlapFigures(Coords a, Coords b, int width, int height) {
	bool xOverlap = checkCoordsInLimits(a.x, b.x, b.x + width) || checkCoordsInLimits(b.x, a.x, a.x + width);
	bool yOverlap = checkCoordsInLimits(a.y, b.y, b.y + height) || checkCoordsInLimits(b.y, a.y, a.y + height);

	return xOverlap && yOverlap;
}
int checkWin(Brick *bricks, int numBricks) {
	int count = 0;
	for (int i = 0; i < numBricks; i++) {
		if (bricks[i].lifes > 0)
			count++;
	}
	if (count == 0)
		return SUCCESS;
	return FAILURE;
}

int checkLost(int *gameActive, Ball *ball, Player *players, int maxPlayers, int paddles_line) {
	if (ball->coords.y <= paddles_line)
		return FAILURE;
	int count = 0;
	for (int i = 0; i < maxPlayers; i++) {
		if (players[i].valid && players[i].playing && !players[i].spectating) {
			if (players[i].lives == 1) {
				players[i].spectating = 1;
			}
			else {
				count++;
			}
			players[i].lives--;
			_tprintf(TEXT("\n[SERVER] %s lost a life.\n"), players[i].username);
		}
	}
	if (count == 0) {
		*gameActive = 0;
	}
	return SUCCESS;
}

int checkBallCoordsOnBoard(Ball ball, Board board) {
	switch (ball.direction) {
	case BALL_DIRECTION_UP_LEFT:
		if (ball.coords.x - 1 == board.coords.x && ball.coords.y - 1 == board.coords.y) { //CANTO SUPERIOR ESQUERDO
			return BALL_DIRECTION_DOWN_RIGHT;
		}
		else if (ball.coords.x - 1 == board.coords.x) {        // CASO A BOLA ESTEJA NO LIMITE DA ESQUERDA
			return BALL_DIRECTION_UP_RIGHT;
		}
		else if (ball.coords.y - 1 == board.coords.y) {  // CASO A BOLA ESTEJA NO LIMITE DE CIMA
			return BALL_DIRECTION_DOWN_LEFT;
		}
		break;
	case BALL_DIRECTION_UP_RIGHT:
		if (ball.coords.x + 1 == board.coords.x + board.x_dimension && ball.coords.y - 1 == board.coords.y) { //CANTO SUPERIOR DIREITO
			return BALL_DIRECTION_DOWN_LEFT;
		}
		else if (ball.coords.x + 1 == board.coords.x + board.x_dimension) {  // CASO A BOLA ESTEJA NO LIMITE DA DIREITA
			return BALL_DIRECTION_UP_LEFT;
		}
		else if (ball.coords.y - 1 == board.coords.y) { // CASO A BOLA ESTEJA NO LIMITE DE CIMA
			return BALL_DIRECTION_DOWN_RIGHT;
		}
		break;
	case BALL_DIRECTION_DOWN_LEFT:
		if (ball.coords.x - 1 == board.coords.x && ball.coords.y + 1 == board.coords.y + board.y_dimension) { //CANTO INFERIOR ESQUERDO
			return BALL_DIRECTION_UP_RIGHT;
		}
		else if (ball.coords.x - 1 == board.coords.x) {
			return BALL_DIRECTION_DOWN_RIGHT;
		}
		else if (ball.coords.y + 1 == board.coords.y + board.y_dimension) { // CASO A BOLA ESTEJA NO LIMITE DE BAIXO
			return BALL_DIRECTION_UP_LEFT;
		}
		break;
	case BALL_DIRECTION_DOWN_RIGHT:
		if (ball.coords.x + 1 == board.coords.x + board.x_dimension && ball.coords.y + 1 == board.coords.y + board.y_dimension) { //CANTO INFERIOR DIREITO
			return BALL_DIRECTION_UP_LEFT;
		}
		else if (ball.coords.x + 1 == board.coords.x + board.x_dimension) { // CASO A BOLA ESTEJA NO LIMITE DA DIREITA
			return BALL_DIRECTION_DOWN_LEFT;
		}
		else if (ball.coords.y + 1 == board.coords.y + board.y_dimension) { // CASO A BOLA ESTEJA NO LIMITE DE BAIXO
			return BALL_DIRECTION_UP_RIGHT;
		}
	}
	return SUCCESS;
}
void addScoreToPlayer(int points, TCHAR *username, Player *players, int maxPlayers) {
	for (int i = 0; i < maxPlayers; i++) {
		if (players[i].valid && players[i].playing && !players[i].spectating && _tcsncmp(username, players[i].username, MAX_NAME) == 0) {
			players[i].score += points;
			_tprintf(TEXT("\n[SERVER] %s scored %d.\n"), players[i].username, points);
			break;
		}
	}
}

int takeOneBrickLife(Brick *brick, Ball ball, Player *players, int maxPlayers) {
	brick->lifes--;
	if (brick->lifes <= 0) {
		if (_tcsncmp(ball.player, TEXT(""), MAX_NAME) != 0) {
			_tprintf(TEXT("\n[SERVER] A brick has been broken by %s.\n"), ball.player);
			addScoreToPlayer(brick->points, ball.player, players, maxPlayers);
		}
		//return SUCCESS;
	}
	return FAILURE;
}

int checkBallCoordsOnBricks(Ball ball, Board *board, Player *players, int maxPlayers) {
	for (int i = 0; i < GAME_INITIAL_NUM_BRICKS; i++) {
		if (board->bricks[i].lifes > 0) {
			for (int j = 0; j < board->bricks[i].width; j++) {
				switch (ball.direction) {
				case BALL_DIRECTION_DOWN_LEFT:
					if (ball.coords.x - 1 == board->bricks[i].coords.x + j && ball.coords.y + 1 == board->bricks[i].coords.y)
						return takeOneBrickLife(&(board->bricks[i]), board->ball, players, maxPlayers);
				case BALL_DIRECTION_DOWN_RIGHT:
					if (ball.coords.x + 1 == board->bricks[i].coords.x + j && ball.coords.y + 1 == board->bricks[i].coords.y)
						return takeOneBrickLife(&(board->bricks[i]), board->ball, players, maxPlayers);
				case BALL_DIRECTION_UP_LEFT:
					if (ball.coords.x - 1 == board->bricks[i].coords.x + j && ball.coords.y - 1 == board->bricks[i].coords.y)
						return takeOneBrickLife(&(board->bricks[i]), board->ball, players, maxPlayers);
				case BALL_DIRECTION_UP_RIGHT:
					if (ball.coords.x + 1 == board->bricks[i].coords.x + j && ball.coords.y - 1 == board->bricks[i].coords.y)
						return takeOneBrickLife(&(board->bricks[i]), board->ball, players, maxPlayers);
				}
			}
		}
	}
	return SUCCESS;
}

int checkBallCoordsOnPaddle(Ball *ball, Paddle paddle, Player *player) {
	switch (ball->direction) {
	case BALL_DIRECTION_DOWN_LEFT:
		for (int k = 0; k < paddle.width; k++) {
			if (ball->coords.x - 1 == paddle.coords.x + k && ball->coords.y + 1 == paddle.coords.y) {
				_tcscpy_s(ball->player, MAX_NAME, player->username);
				_tprintf(TEXT("\n[SERVER] %s touched the ball.\n"), ball->player);
				return FAILURE;
			}
		}
		break;
	case BALL_DIRECTION_DOWN_RIGHT:
		for (int j = 0; j < paddle.width; j++) {
			if (ball->coords.x + 1 == paddle.coords.x + j && ball->coords.y + 1 == paddle.coords.y) {
				_tcscpy_s(ball->player, MAX_NAME, player->username);
				_tprintf(TEXT("\n[SERVER] %s touched the ball.\n"), ball->player);
				return FAILURE;
			}
		}
		break;
	}
	return SUCCESS;
}

void moveBall(Ball *ball, Board *board, Player *players, int maxPlayers) {
	int resCheckOnBoard = -1;
	int resCheckOnPaddle = -1;
	int resCheckOnBricks = -1;

	while ((resCheckOnBoard != 0 && resCheckOnPaddle != 0) ||
		(resCheckOnBoard != 0 && resCheckOnBricks != 0) ||
		(resCheckOnPaddle != 0 && resCheckOnBricks != 0))
	{
		resCheckOnBoard = checkBallCoordsOnBoard(*ball, *board);

		if (resCheckOnBoard > 0) {
			ball->direction = resCheckOnBoard;
		}

		resCheckOnPaddle = 0;

		for (int i = 0; i < maxPlayers; i++) {
			if (players[i].valid && players[i].playing) {
				if (checkBallCoordsOnPaddle(ball, players[i].paddle, &(players[i])) == 1)
					resCheckOnPaddle = 1;
			}
		}

		if (resCheckOnPaddle > 0) {
			switch (ball->direction) {
			case BALL_DIRECTION_DOWN_LEFT:
				ball->direction = BALL_DIRECTION_UP_LEFT;
				break;
			case BALL_DIRECTION_DOWN_RIGHT:
				ball->direction = BALL_DIRECTION_UP_RIGHT;
				break;
			}
		}

		resCheckOnBricks = checkBallCoordsOnBricks(*ball, board, players, maxPlayers);

		if (resCheckOnBricks > 0) {
			switch (ball->direction) {
			case BALL_DIRECTION_UP_LEFT:
				ball->direction = BALL_DIRECTION_DOWN_LEFT;
				break;
			case BALL_DIRECTION_UP_RIGHT:
				ball->direction = BALL_DIRECTION_DOWN_RIGHT;
				break;
			case BALL_DIRECTION_DOWN_LEFT:
				ball->direction = BALL_DIRECTION_UP_LEFT;
				break;
			case BALL_DIRECTION_DOWN_RIGHT:
				ball->direction = BALL_DIRECTION_UP_RIGHT;
			}
		}

	}

	switch (ball->direction) {
	case BALL_DIRECTION_UP_LEFT:
		ball->coords.x--;
		ball->coords.y--;
		ball->direction = BALL_DIRECTION_UP_LEFT;
		break;
	case BALL_DIRECTION_UP_RIGHT:
		ball->coords.x++;
		ball->coords.y--;
		ball->direction = BALL_DIRECTION_UP_RIGHT;
		break;
	case BALL_DIRECTION_DOWN_LEFT:
		ball->coords.x--;
		ball->coords.y++;
		ball->direction = BALL_DIRECTION_DOWN_LEFT;
		break;
	case BALL_DIRECTION_DOWN_RIGHT:
		ball->coords.x++;
		ball->coords.y++;
		ball->direction = BALL_DIRECTION_DOWN_RIGHT;
		break;
	}
}

bool checkCoordsNearness(int x1, int x2) {
	return x1 == x2;
}

void moveBricks(Brick *bricks, int num_bricks, Board board, int direction) {

	switch (direction) {
	case 1:
		for (int i = 0; i < num_bricks; i++) {
			if (checkCoordsNearness(bricks[i].coords.x + bricks[i].width - 1, board.coords.x + board.x_dimension - 1) == false) {
				bricks[i].coords.x++;
			}
		}
		return;
	case 0:
		for (int i = 0; i < num_bricks; i++) {
			if (checkCoordsNearness(bricks[i].coords.x - 1, board.coords.x) == false) {
				bricks[i].coords.x--;
			}
		}
		return;
	}
}

int movePaddle(Paddle * paddle, Board board, int moveDirection) {
	// Temos acesso ao board para saber os limites do mesmo
	switch (moveDirection) {
	case ACTION_MOVE_RIGHT:
		if (checkCoordsNearness(paddle->coords.x + paddle->width - 1, board.coords.x + board.x_dimension - 1) == false) {
			paddle->coords.x++;
			return SUCCESS;
		}
		break;
	case ACTION_MOVE_LEFT:
		if (checkCoordsNearness(paddle->coords.x - 1, board.coords.x) == false) {
			paddle->coords.x--;
			return SUCCESS;
		}
		break;
	}
	return FAILURE;
}