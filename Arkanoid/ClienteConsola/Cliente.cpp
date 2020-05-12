
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "Cliente.h"

// Functions
int makeLogin(void);
void mainMenu(void);
int createMutexs(void);
int createThreads(void);
void loadDefaultKeys(void);

// Global Variables
GameData game;
Configurations configs;
HWND hWnd;
int run, inGame, spectate;
HANDLE hMutexData, pThreadGetGame, pThreadGetThings, console;

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	_tprintf(TEXT("\n[CLIENT SIDE]\n\n"));

	Answer answer = PingServer();
	if (answer.type == ANSWER_PING_SERVER_ONLINE)
		_tprintf(TEXT("We have server!\n"));
	else {
		_tprintf(TEXT("The server is disconnected or hasn't responded. Please try again later.\n"));
		system("pause");
		return FAILURE;
	}

	if (createMutexs() == FAILURE) {
		return FAILURE;
	}

	if (makeLogin() == FAILURE) {
		_tprintf(TEXT("\n[CLIENT] Can't connect to the server.\n"));
		return FAILURE;
	}

	loadDefaultKeys();
	_tprintf(TEXT("\n[CLIENT] Connected to the server.\n"));

	run = 1;

	mainMenu();

	SendRequest(configs.username, ACTION_SHUTDOWN);

	return SUCCESS;
}

int requestUsername(TCHAR* username) {
	_tprintf(TEXT("\nPlease enter your username: "));
	if (_fgetts(username, MAX_NAME, stdin) != NULL) {
		username[_tcslen(username) - 1] = '\0';
		return SUCCESS;
	}
	return FAILURE;
}

int makeLogin(void) {
	TCHAR username[MAX_NAME];

	if (requestUsername(username) == FAILURE) {
		_tprintf(TEXT("\n[CLIENT] Invalid username.\n"));
		return FAILURE;
	}
	_tcscpy_s(configs.username, MAX_NAME, username);
	Answer answer;
	answer = Login(username);
	return answer.type;
}

void loadDefaultKeys(void) {
	configs.move_right = DEFAULT_KEY_MOVE_RIGHT;
	configs.move_left = DEFAULT_KEY_MOVE_LEFT;
}

int createMutexs(void) {
	hMutexData = CreateMutex(NULL, FALSE, NULL);
	if (hMutexData == NULL) {
		_tprintf(TEXT("\n[CLIENT] Create mutex failed and error number %d \n"), GetLastError());
		return FAILURE;
	}
	_tprintf(TEXT("\n[CLIENT] Mutex for data has been created.\n"));

	return SUCCESS;
}

void gotoxy(int x, int y) {
	static HANDLE hStdout = NULL;
	COORD coord;
	coord.X = x;
	coord.Y = y;
	if (!hStdout)
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hStdout, coord);
}

void setCursor(bool visible) {
	// set bool visible = 0 - invisible, bool visible = 1 - visible
	DWORD	size = 20;
	// default cursor size Changing to numbers from 1 to 20, decreases cursor width
	CONSOLE_CURSOR_INFO lpCursor;
	lpCursor.bVisible = visible;
	lpCursor.dwSize = size;
	SetConsoleCursorInfo(console, &lpCursor);
}

void showGame(void) {
	// Oculta o Cursor
	setCursor(false);

	//// Limpa Ecrã
	//for (int i = game.board.coords.x; i <= game.board.x_dimension; i++) {
	//	for (int j = game.board.coords.y; j <= game.board.x_dimension; j++) {
	//		gotoxy(i, j);
	//		_tprintf(TEXT(" "));
	//	}
	//}
	system("cls");

	// Imprimir Tabuleiro - Limites das linhas
	for (int k = 0; k < game.board.y_dimension; k++) {
		gotoxy(0, k);
		_tprintf(TEXT("@"));
		gotoxy(game.board.x_dimension, k);
		_tprintf(TEXT("@"));
	}

	// Imprimir Tabuleiro - Limites das colunas
	for (int j = 0; j < game.board.x_dimension + 1; j++) {  // + 1 para não ficar buraco no fundo
		gotoxy(j, 0);
		_tprintf(TEXT("@"));
		gotoxy(j, game.board.y_dimension);
		_tprintf(TEXT("@"));
	}

	// Imprimir Paddles dos Jogadores
	// TODO COLOCAR CONFIG MAX PLAYERS NO GAME DATA
	for (int i = 0; i < GAME_MAX_PLAYERS; i++) {
		if (game.players[i].valid && game.players[i].playing) {
			for (int j = 0; j < game.players[i].paddle.width; j++) {
				gotoxy(game.players[i].paddle.coords.x + j, game.players[i].paddle.coords.y);
				_tprintf(TEXT("%c"), 9608);
			}
		}
	}

	// Imprimir Bola
	gotoxy(game.board.ball.coords.x, game.board.ball.coords.y);
	_tprintf(TEXT("O"));

	// Colocar o ponteiro fora do tabuleiro
	gotoxy(0, game.board.coords.y + game.board.y_dimension + 2);

	// Mostra o Cursor
	setCursor(true);
}

DWORD WINAPI getGame(void) {
	do {
		WaitForSingleObject(hMutexData, INFINITE);
		game = ReceiveBroadcast();
		if (!game.run) {
			_tprintf(TEXT("\n[CLIENT] The server disconnected.\n"));
			inGame = 0;
		}

		if (!game.gameActive) {
			_tprintf(TEXT("\n[CLIENT] The ongoing game has ended.\n"));
			inGame = 0;
		}

		ReleaseMutex(hMutexData);
		showGame();
	} while (game.gameActive && inGame);

	return SUCCESS;
}

int createThreads(void) {
	pThreadGetGame = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getGame, NULL, 0, NULL);
	if (pThreadGetGame == NULL) {
		_tprintf(TEXT("\n[CLIENT] pThreadGetGame error %d.\n"), GetLastError());
		return FAILURE;
	}
	_tprintf(TEXT("\n[CLIENT] Thread for get the game has been created.\n"));
	return SUCCESS;
}

void redefineKeys(void) {
	int opt;
	do {
		_tprintf(TEXT("\nHello %s, which key do you want to change?\n"), configs.username);
		_tprintf(TEXT("\t1- Move Right (%c)\n"), configs.move_right);
		_tprintf(TEXT("\t2- Move Left (%c)\n"), configs.move_left);
		_tprintf(TEXT("\t0- Finish configurations\n"));
		_tprintf(TEXT("Option: "));
		_tscanf_s(TEXT("%d"), &opt);

		int new_key;

		switch (opt) {
		case 1:
			_tprintf(TEXT("New right key: "));
			new_key = _gettch();
			_tprintf(TEXT("\nNew key chosen: %c"), new_key);
			if (new_key != configs.move_left)
				configs.move_right = new_key;
			break;
		case 2:
			_tprintf(TEXT("New left key: "));
			new_key = _gettch();
			_tprintf(TEXT("\nNew key chosen: %c"), new_key);
			if (new_key != configs.move_right)
				configs.move_left = new_key;
			break;
		}
	} while (opt != 0);

}

int readAction(void) {
	int key = _gettch();
	if (key == configs.move_right)
		return ACTION_MOVE_RIGHT;
	if (key == configs.move_left)
		return ACTION_MOVE_LEFT;
	if (key == DEFAULT_KEY_QUIT)
		return ACTION_QUIT_GAME;
	return -1;
}

void startGame(void) {
	Answer answer = PingServer();
	if (answer.type == ANSWER_PING_SERVER_OFFLINE) {
		_tprintf(TEXT("\n[CLIENT] The server disconnected.\n"));
		return;
	}

	answer = JoinGame(configs.username);
	if (answer.type == ANSWER_JOIN_GAME_TO_PLAY) {
		_tprintf(TEXT("You will be playing in the next game!\n"));
		spectate = 0;
	}
	else {
		_tprintf(TEXT("You will be spectating!\n"));
		spectate = 1;
	}

	inGame = 1;

	if (createThreads() == FAILURE) {
		return;
	}

	_tprintf(TEXT("Waiting for the server to respond...\n"));
	WaitForGameToBegin();

	int action;

	while (inGame) {
		action = readAction();
		if (action != -1) {
			if (action == ACTION_QUIT_GAME) {
				WaitForSingleObject(hMutexData, INFINITE);
				inGame = 0;
				ReleaseMutex(hMutexData);
				SendRequest(configs.username, action);
			}
			else {
				if (!spectate) {
					SendRequest(configs.username, action);
				}
			}
		}
	}
	WaitForSingleObject(pThreadGetGame, INFINITE);
}
void showTopRanking(void) {
	Answer answer = PingServer();
	if (answer.type == ANSWER_PING_SERVER_OFFLINE) {
		_tprintf(TEXT("\n[CLIENT] The server disconnected.\n"));
		return;
	}

	answer = GetTopTen();

	_tprintf(TEXT("\n\nArkanoid - Top Ranking\n\n"));
	_tprintf(TEXT("\tRank\tUsername\t\t\t\tScore\n"));
	for (int i = 0; i < GAME_TOP_RANK; i++) {
		_tprintf(TEXT("\t%2d\t%-35s\t%06d\n"), i + 1, answer.ranking[i].username, answer.ranking[i].score);
	}
}

void mainMenu(void) {
	int opt;
	while (run) {
		_tprintf(TEXT("\nHello %s, which action do you want to do?\n"), configs.username);
		_tprintf(TEXT("\t1- Play\n"));
		_tprintf(TEXT("\t2- Change hotkeys\n"));
		_tprintf(TEXT("\t3- Top Ranking\n"));
		_tprintf(TEXT("\t4- Quit\n"));

		_tscanf_s(TEXT("%d"), &opt);

		switch (opt) {
		case 1:
			_tprintf(TEXT("Starting game...\n"));
			startGame();
			break;
		case 2:
			_tprintf(TEXT("Redefine keys...\n"));
			redefineKeys();
			break;
		case 3:
			_tprintf(TEXT("Showing top ranking...\n"));
			showTopRanking();
			break;
		case 4:
			run = 0;
		}
	}
}