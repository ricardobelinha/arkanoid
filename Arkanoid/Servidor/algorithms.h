#pragma once
#include "../DLL/structs.h"

bool checkCoordsInLimits(int value, int min, int max);
bool overlapFigures(Coords a, Coords b, int width, int height);
void moveBall(Ball *ball, Board *board, Player *players, int maxPlayers);
int movePaddle(Paddle *paddle, Board board, int moveDirection);
void moveBricks(Brick *bricks, int num_bricks, Board board, int direction);
int checkLost(int *gameActive, Ball *ball, Player *players, int maxPlayers, int paddles_line);
int checkWin(Brick *bricks, int numBricks);