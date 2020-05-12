#pragma once
#include <tchar.h>

#define SUCCESS 0
#define FAILURE 1

#define MAX_NAME 25
#define MAX_CMD 50
#define MAX_BUFFER 512
#define SHARED_DIRECTORY TEXT("c:\\server-dir")

#define GAME_MAX_PLAYERS 13 // NAO ALTERAR, POR CAUSA DAS CORES NO CLIENTE GRÁFICO
#define GAME_MAX_LEVELS 5
#define GAME_INITIAL_LIVES 3
#define GAME_INITIAL_PADDLE_WIDTH 15
#define GAME_INITIAL_NUM_BRICKS 20
#define GAME_INITIAL_NUM_SPEED_UPS 3
#define GAME_INITIAL_NUM_SLOW_DOWNS 3
#define GAME_INITIAL_NUM_EXTRA_LIVES 3
#define GAME_INITIAL_NUM_TRIPLES 3
#define GAME_BOARD_DIMENSION_X 150
#define GAME_BOARD_DIMENSION_Y 75
#define GAME_PADDLE_INITIAL_LINE (GAME_BOARD_DIMENSION_Y - 2)
#define GAME_BALL_INITIAL_LINE (GAME_PADDLE_INITIAL_LINE - 1)
#define GAME_BONUS_CHANCE_SPEED_UPS 3
#define GAME_BONUS_CHANCE_SLOW_DOWNS 3
#define GAME_BONUS_CHANCE_EXTRA_LIVES 3
#define GAME_BONUS_CHANCE_TRIPLES 0
#define GAME_DURATION_SPEED_UPS 3
#define GAME_DURATION_SLOW_DOWNS 3
#define GAME_DURATION_TRIPLES 3
#define GAME_MOVEMENT_SPEED_PX 5
#define GAME_TOP_RANK 10

#define ACTION_LOGIN 0
#define ACTION_SHUTDOWN 1
// #define ACTION_SPACE_BAR 2
#define ACTION_MOVE_RIGHT 3
#define ACTION_MOVE_LEFT 4
#define ACTION_GET_TOP_10 5
#define ACTION_JOIN_GAME 6
#define ACTION_QUIT_GAME 7
#define ACTION_PING_SERVER 8

#define ANSWER_LOGIN_SUCCESS 0
#define ANSWER_LOGIN_FAILURE 1
#define ANSWER_JOIN_GAME_TO_PLAY 2
#define ANSWER_JOIN_GAME_TO_SPECTATE 3
#define ANSWER_PING_SERVER_ONLINE 4
#define ANSWER_PING_SERVER_OFFLINE 5
#define ANSWER_GET_TOP_10 6
#define ANSWER_REMOTE_COMMUNICATION_FAILURE 7

#define BALL_DIRECTION_UP_LEFT 1
#define BALL_DIRECTION_UP_RIGHT 2
#define BALL_DIRECTION_DOWN_LEFT 3
#define BALL_DIRECTION_DOWN_RIGHT 4

#define BRICK_TYPE_NORMAL 0
#define BRICK_NORMAL_WIDTH 10
#define BRICK_NORMAL_POINTS 100
#define BRICK_NORMAL_MAX_LIVES 1	
#define BRICK_TYPE_RESISTANT 1
#define BRICK_RESISTANT_WIDTH 10
#define BRICK_RESISTANT_POINTS 200
#define BRICK_RESISTANT_MIN_LIVES 2
#define BRICK_RESISTANT_MAX_LIVES 4
#define BRICK_TYPE_MAGIC 2
#define BRICK_MAGIC_WIDTH 10
#define BRICK_MAGIC_POINTS 300
#define BRICK_MAGIC_MAX_LIVES 1

#define BONUS_TYPE_SPEED_UP 0
#define BONUS_TYPE_SLOW_DOWN 1
#define BONUS_TYPE_EXTRA_LIFE 2
#define BONUS_TYPE_TRIPLE 3
#define BONUS_TYPE_INCREASE_PADDLE_WIDTH 4
#define BONUS_TYPE_DECREASE_PADDLE_WIDTH 5

typedef struct {
	int x;
	int y;
} Coords;

typedef struct {
	int width;
	Coords coords;
} Paddle;

typedef struct {
	int valid; // 1 - válido
	int playing; // 1 - a jogar
	int spectating; // 1 - a assistir
	TCHAR username[MAX_NAME];
	Paddle paddle;
	int score;
	int lives;
} Player;

typedef struct {
	Coords coords;
	int direction;
	int speed;
	TCHAR player[MAX_NAME];
} Ball;

typedef struct {
	int type;
	int width;
	Coords coords;
	int points;
	int lifes;
} Brick;

typedef struct {
	Coords coords;
	int x_dimension;
	int y_dimension;
	Brick bricks[GAME_INITIAL_NUM_BRICKS];
	int numBricks;
	int bricks_speed;
	Ball ball;
} Board;

typedef struct {
	int max_players;
	int max_levels;
	int initial_lives;
	int initial_paddle_width;
	int paddle_initial_line;
	int ball_initial_line;
	int brick_normal_max_lives;
	int brick_normal_width;
	int brick_normal_points;
	int brick_resistant_max_lives;
	int brick_resistant_width;
	int brick_resistant_points;
	int brick_magic_max_lives;
	int brick_magic_width;
	int brick_magic_points;
} Configs;

typedef struct {
	TCHAR username[MAX_NAME];
	int score;
} Ranking;

typedef struct {
	int type;
	TCHAR username[MAX_NAME];
} Request;

typedef struct {
	int type;
	Ranking ranking[GAME_TOP_RANK];
} Answer;

//typedef union {
//	int message_type;
//	Ranking ranking[GAME_TOP_RANK];
//} Response;

typedef struct {
	Brick bricks[GAME_INITIAL_NUM_BRICKS];
	int ball_speed;
	int num_bricks;
	int bricks_speed;
}Level;

typedef struct {
	int run; // Estado do servidor
	int gameActive; // Estado do jogo
	int win; // ESTADO FINAL DE JOGO
	Player players[GAME_MAX_PLAYERS];
	Board board;
	Configs configs;
	int level;
} GameData;

