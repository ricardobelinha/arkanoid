#define _CRT_SECURE_NO_WARNINGS
#include "Server.h"
#include "../DLL/DLL.h"
#include <strsafe.h>
#include <aclapi.h>
#include "levels.h"

// Service NT
// Internal name of the service 
#define SERVICE_NAME L"CppWindowsService" 
// Displayed name of the service 
#define SERVICE_DISPLAY_NAME L"CppWindowsService Sample Service" 
// Service start options. 
#define SERVICE_START_TYPE SERVICE_DEMAND_START 
// List of service dependencies - "dep1\0dep2\0\0" 
#define SERVICE_DEPENDENCIES L"" 
// The name of the account under which the service should run 
#define SERVICE_ACCOUNT L"NT AUTHORITY\\LocalService" 
// The password to the service account name 
#define SERVICE_PASSWORD NULL 

// Functions
void initializeStructures(void);
int createSharedDirectory(void);
void gameConfigurationWithFile(TCHAR* filename);
void configureLevels(void);
void readCommands(void);
int createMutexs(void);
int createThreads(void);
void endGame(void);

// Global Variables
GameData game;
Ranking ranking[GAME_TOP_RANK];
Level levels[GAME_MAX_LEVELS];
SECURITY_ATTRIBUTES sa;
HANDLE hMutexData;
HANDLE pThreadBallMovement, pThreadPlayerRequests, pThreadBricksMovement;
int inServiceNT = 0;

int _tmain(int argc, LPTSTR argv[]) {
	_tprintf(TEXT("[SERVER SIDE]\n\n"));

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	initializeStructures();

	if (createSharedDirectory() == FAILURE) {
		return FAILURE;
	}

	if (argc > 2) {
		_tprintf(TEXT("\n[SERVER] Sintax: .\\server <config filename>\n"));
		return FAILURE;
	}
	else if (argc == 2) {
		gameConfigurationWithFile(argv[1]);
	}

	game.run = 1;
	configureLevels();

	if (createMutexs() == FAILURE || createThreads() == FAILURE) {
		return FAILURE;
	}

	_tprintf(TEXT("\n[SERVER] Listening to remote clients.\n"));
	if (initializeRemoteCommunication() == FAILURE)
		_tprintf(TEXT("\n[SERVER] Remote not Working.\n"));
	//waitForRemoteClients(&(game.run));

	readCommands();

	//WaitForSingleObject(pThreadPlayerRequests, INFINITE);
	return SUCCESS;
}

void initializeStructures(void) {
	// Game
	game.run = 0;
	game.gameActive = 0;
	game.board.numBricks = 0;
	game.level = 1;
	for (int i = 0; i < GAME_MAX_PLAYERS; i++) {
		game.players[i].valid = 0;
		game.players[i].playing = 0;
		_tcscpy_s(game.players[i].username, MAX_NAME, TEXT(""));
	}

	game.board.coords.x = 0;
	game.board.coords.y = 0;
	game.board.x_dimension = GAME_BOARD_DIMENSION_X;
	game.board.y_dimension = GAME_BOARD_DIMENSION_Y;

	// Configs
	game.configs.max_players = GAME_MAX_PLAYERS;
	game.configs.max_levels = GAME_MAX_LEVELS;
	game.configs.initial_lives = GAME_INITIAL_LIVES;
	game.configs.initial_paddle_width = GAME_INITIAL_PADDLE_WIDTH;
	game.configs.paddle_initial_line = GAME_PADDLE_INITIAL_LINE;
	game.configs.ball_initial_line = GAME_BALL_INITIAL_LINE;
	game.configs.brick_normal_points = BRICK_NORMAL_POINTS;
	game.configs.brick_normal_width = BRICK_NORMAL_WIDTH;
	game.configs.brick_normal_max_lives = BRICK_NORMAL_MAX_LIVES;
	game.configs.brick_resistant_points = BRICK_RESISTANT_POINTS;
	game.configs.brick_resistant_width = BRICK_RESISTANT_WIDTH;
	game.configs.brick_resistant_max_lives = BRICK_RESISTANT_MAX_LIVES;
	game.configs.brick_magic_points = BRICK_MAGIC_POINTS;
	game.configs.brick_magic_width = BRICK_MAGIC_WIDTH;
	game.configs.brick_magic_max_lives = BRICK_MAGIC_MAX_LIVES;

	// Highscore
	// TODO USAR SCORE DO JOGO JÁ ESTÁ FEITO
	int score = 0;
	for (int i = 0; i < GAME_TOP_RANK; i++) {
		_tcscpy_s(ranking[i].username, MAX_NAME, TEXT(""));
		ranking[i].score = score;
	}

	if (initializeRankingInRegistry(ranking) == FAILURE) {
		_tprintf(TEXT("[SERVER] Ranking data is already in the system.\n"));
		getRankingData(ranking);
	}
	else {
		_tprintf(TEXT("[SERVER] Ranking was created successfully at Windows Registry.\n"));
	}
}

// Buffer clean up routine
void Cleanup(PSID pEveryoneSID, PSID pAdminSID, PACL pACL, PSECURITY_DESCRIPTOR pSD) {
	if (pEveryoneSID)
		FreeSid(pEveryoneSID);
	if (pAdminSID)
		FreeSid(pAdminSID);
	if (pACL)
		LocalFree(pACL);
	if (pSD)
		LocalFree(pSD);
}

int Seguranca(SECURITY_ATTRIBUTES * sa) {
	PSECURITY_DESCRIPTOR pSD;
	PACL pAcl;
	EXPLICIT_ACCESS ea;
	PSID pEveryoneSID = NULL, pAdminSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	TCHAR str[256];

	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (pSD == NULL) {
		_tprintf(TEXT("\n[SERVER] Error on local allocation!\n"));
		return FAILURE;
	}
	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
		_tprintf(TEXT("\n[SERVER] Error on initialize security descriptor!\n"));
		return FAILURE;
	}

	// Create a well-known SID for the Everyone group.
	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
	{
		_stprintf_s(str, 256, TEXT("\n[SERVER] Allocate and initialize Sid error %u!\n"), GetLastError());
		_tprintf(str);
		Cleanup(pEveryoneSID, pAdminSID, NULL, pSD);
	}
	else
		_tprintf(TEXT("\n[SERVER] Allocate and initialize Sid for the Everyone group is OK!\n"));

	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));

	ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;

	if (SetEntriesInAcl(1, &ea, NULL, &pAcl) != ERROR_SUCCESS) {
		_tprintf(TEXT("\n[SERVER] Erro on setting entries in acl!\n"));
		return FAILURE;
	}

	if (!SetSecurityDescriptorDacl(pSD, TRUE, pAcl, FALSE)) {
		_tprintf(TEXT("\n[SERVER] Error on setting security descriptor!\n"));
		return FAILURE;
	}

	sa->nLength = sizeof(*sa);
	sa->lpSecurityDescriptor = pSD;
	sa->bInheritHandle = TRUE;
	return SUCCESS;
}

int createSharedDirectory(void) {
	if (Seguranca(&sa) == FAILURE) {
		return FAILURE;
	}
	if (!CreateDirectory(SHARED_DIRECTORY, &sa)) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			_tprintf(TEXT("\n[SERVER] Directory %s already exists.\n"), SHARED_DIRECTORY);
			return SUCCESS;
		}
		_tprintf(TEXT("\n[SERVER] Error on shared directory creation!\n"));
		return FAILURE;
	}
	_tprintf(TEXT("\n[SERVER] Directory %s successfuly created.\n"), SHARED_DIRECTORY);
	return SUCCESS;
}

bool playerCanJoinGame(void) {
	return !game.gameActive;
}

void gameConfigurationWithFile(TCHAR* filename) {
	FILE* file;
	TCHAR message[MAX_BUFFER];
	int value;
	int result;

	result = _tfopen_s(&file, filename, TEXT("rt"));

	if (result != 0) {
		_tprintf(TEXT("\n[SERVER] Error on loading the configuration file. Defaults loaded.\n"));
		return;
	}

	while (_ftscanf(file, TEXT(" %s %d"), message, &value) == 2) {
		if (_tcsncmp(message, TEXT("max_players"), 11) == 0) {
			if (value > 0 && value < GAME_MAX_PLAYERS) {
				game.configs.max_players = value;
				_tprintf(TEXT("\n[SERVER] MAX_PLAYERS has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("initial_lives"), 13) == 0) {
			if (value > 0 && value < GAME_INITIAL_LIVES) {
				game.configs.initial_lives = value;
				_tprintf(TEXT("\n[SERVER] INITIAL_LIVES has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("initial_paddle_width"), 20) == 0) {
			if (value > 0 && value < GAME_INITIAL_PADDLE_WIDTH) {
				game.configs.initial_paddle_width = value;
				_tprintf(TEXT("\n[SERVER] INITIAL_PADDLE_WIDTH has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("max_levels"), 10) == 0) {
			if (value > 0 && value < GAME_MAX_LEVELS) {
				game.configs.max_levels = value;
				_tprintf(TEXT("\n[SERVER] MAX_LEVELS has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("paddle_initial_line"), 19) == 0) {
			if (value > 0 && value < GAME_PADDLE_INITIAL_LINE) {
				game.configs.paddle_initial_line = value;
				_tprintf(TEXT("\n[SERVER] PADDLE_INITIAL_LINE has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("ball_initial_line"), 17) == 0) {
			if (value > 0 && value < GAME_BALL_INITIAL_LINE) {
				game.configs.ball_initial_line = value;
				_tprintf(TEXT("\n[SERVER] BALL_INITIAL_LINE has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_normal_width"), 18) == 0) {
			if (value > 0 && value < BRICK_NORMAL_WIDTH) {
				game.configs.brick_normal_width = value;
				_tprintf(TEXT("\n[SERVER] BRICK_NORMAL_WIDTH has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_normal_points"), 19) == 0) {
			if (value >= 0 && value < BRICK_NORMAL_POINTS) {
				game.configs.brick_normal_points = value;
				_tprintf(TEXT("\n[SERVER] BRICK_NORMAL_POINTS has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_normal_max_lives"), 22) == 0) {
			if (value > 0 && value < BRICK_NORMAL_MAX_LIVES) {
				game.configs.brick_normal_max_lives = value;
				_tprintf(TEXT("\n[SERVER] BRICK_NORMAL_MAX_LIVES has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_resistant_width"), 21) == 0) {
			if (value > 0 && value < BRICK_RESISTANT_WIDTH) {
				game.configs.brick_resistant_width = value;
				_tprintf(TEXT("\n[SERVER] BRICK_RESISTANT_WIDTH has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_resistant_points"), 22) == 0) {
			if (value >= 0 && value < BRICK_RESISTANT_POINTS) {
				game.configs.brick_resistant_points = value;
				_tprintf(TEXT("\n[SERVER] BRICK_RESISTANT_POINTS has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_resistant_max_lives"), 25) == 0) {
			if (value > BRICK_RESISTANT_MIN_LIVES && value < BRICK_RESISTANT_MAX_LIVES) {
				game.configs.brick_resistant_max_lives = value;
				_tprintf(TEXT("\n[SERVER] BRICK_RESISTANT_MAX_LIVES has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_magic_width"), 17) == 0) {
			if (value > 0 && value < BRICK_MAGIC_WIDTH) {
				game.configs.brick_magic_width = value;
				_tprintf(TEXT("\n[SERVER] BRICK_MAGIC_WIDTH has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_magic_points"), 18) == 0) {
			if (value >= 0 && value < BRICK_MAGIC_POINTS) {
				game.configs.brick_magic_points = value;
				_tprintf(TEXT("\n[SERVER] BRICK_MAGIC_POINTS has been updated to %d.\n"), value);
			}
		}
		else if (_tcsncmp(message, TEXT("brick_magic_max_lives"), 21) == 0) {
			if (value > 0 && value < BRICK_MAGIC_MAX_LIVES) {
				game.configs.brick_magic_max_lives = value;
				_tprintf(TEXT("\n[SERVER] BRICK_MAGIC_MAX_LIVES has been updated to %d.\n"), value);
			}
		}
	}
	fclose(file);
}

void configureLevels(void) {
	configLevelOne(&(levels[0]), game.configs);
	configLevelTwo(&(levels[1]), game.configs);
	configLevelThree(&(levels[2]), game.configs);
	configLevelFour(&(levels[3]), game.configs);
	configLevelFive(&(levels[4]), game.configs);
}

DWORD WINAPI ThreadPlayerRequests(void) {
	Request request;

	while (game.run) {
		request = ReceiveRequest();
		WaitForSingleObject(hMutexData, INFINITE);
		switch (request.type) {
		case ACTION_LOGIN:
			if (loginPlayerToServer(game.players, request.username, game.configs.max_players) == SUCCESS) {
				_tprintf(TEXT("\n[SERVER] %s connected to the server.\n"), request.username);
				SendAnswer(ANSWER_LOGIN_SUCCESS);
			}
			else {
				_tprintf(TEXT("\n[SERVER] %s can't connect because the server is full.\n"), request.username);
				SendAnswer(ANSWER_LOGIN_FAILURE);
			}
			break;
		case ACTION_JOIN_GAME:
			if (playerCanJoinGame()) {
				loginPlayerToPlayGame(game.players, request.username, game.configs.max_players);
				_tprintf(TEXT("\n[SERVER] %s has been added to the game.\n"), request.username);
				SendAnswer(ANSWER_JOIN_GAME_TO_PLAY);
			}
			else {
				_tprintf(TEXT("\n[SERVER] %s can't join because there's a game running already.\n"), request.username);
				loginPlayerToSpectateGame(game.players, request.username, game.configs.max_players);
				SendAnswer(ANSWER_JOIN_GAME_TO_SPECTATE);
			}
			break;
		case ACTION_QUIT_GAME:
			if (logoutPlayerFromGame(game.players, request.username, game.configs.max_players) == SUCCESS) {
				_tprintf(TEXT("\n[SERVER] %s disconnected from the game.\n"), request.username);
				if (getNumberOfPlayersOnlineFromGame(game.players, game.configs.max_players) == 0) {
					_tprintf(TEXT("\n[SERVER] The game will end because there are no more players.\n"));
					endGame();
				}
			}
			break;
		case ACTION_SHUTDOWN:
			if (logoutPlayerFromServer(game.players, request.username, game.configs.max_players) == SUCCESS) {
				_tprintf(TEXT("\n[SERVER] %s disconnected from the server.\n"), request.username);
			}
			else
				_tprintf(TEXT("\n[SERVER] The was an error while disconnecting %s from the server.\nThe user may have already disconnected.\n"), request.username);
			break;
		case ACTION_MOVE_LEFT:
		case ACTION_MOVE_RIGHT:
			movePlayerPaddle(game.players, request.username, game.configs.max_players, game.board, request.type);
			//if (movePlayerPaddle(game.players, request.username, game.configs.max_players, game.board, request.type) == SUCCESS) {
			//	// TODO PRINT MOVE RIGHT AND LEFT TURN [ON] / [OFF]
			//	if (request.type == ACTION_MOVE_RIGHT)
			//		_tprintf(TEXT("\n[SERVER] %s's paddle moved to the right.\n"), request.username);
			//	else
			//		_tprintf(TEXT("\n[SERVER] %s's paddle moved to the left.\n"), request.username);
			//}
			//else
			//	_tprintf(TEXT("\n[SERVER] %s's paddle stayed in the same location.\n"), request.username);
			break;
		case ACTION_GET_TOP_10:
			WaitForSingleObject(hMutexData, INFINITE);
			SendTop10(ranking);
			ReleaseMutex(hMutexData);
			_tprintf(TEXT("\n[SERVER] Sent top 10.\n"));
			break;
		case ACTION_PING_SERVER:
			SendAnswer(ANSWER_PING_SERVER_ONLINE);
			break;
		}
		ReleaseMutex(hMutexData);
	}

	return SUCCESS;
}

void resetBallPosition(void) {
	game.board.ball.coords.x = GAME_BOARD_DIMENSION_X / 2;
	game.board.ball.coords.y = game.configs.ball_initial_line;
	_tcscpy_s(game.board.ball.player, MAX_NAME, TEXT(""));
	game.board.ball.direction = (rand() % 1) + 1;
}

void resetPaddlesPosition(void) {
	int previousCoordsX = GAME_BOARD_DIMENSION_X / (getNumberOfPlayersOnlineFromGame(game.players, game.configs.max_players) * 2), previousWidth = 0;
	for (int i = 0; i < game.configs.max_players; i++) {
		game.players[i].paddle.coords.x = previousCoordsX + previousWidth - (game.players[i].paddle.width / 2);
		game.players[i].paddle.coords.y = game.configs.paddle_initial_line;
		previousCoordsX = game.players[i].paddle.coords.x;
		previousWidth = game.players[i].paddle.width;
	}
}


void addToRanking(int position, Player newPlayer) {
	int i;
	for (i = GAME_TOP_RANK - 1; i > position; i--) {
		ranking[i] = ranking[i - 1];
	}
	ranking[i].score = newPlayer.score;
	_tcscpy_s(ranking[i].username, MAX_NAME, newPlayer.username);
}

void updateRanking(void) {
	int i, j;
	for (i = 0; i < game.configs.max_players; i++) {
		if (game.players[i].valid && game.players[i].score > 0) {
			_tprintf(TEXT("\n[SERVER] %s scored a total of %d points.\n"), game.players[i].username, game.players[i].score);
			for (j = 0; j < GAME_TOP_RANK; j++) {
				if (ranking[j].score < game.players[i].score) {
					addToRanking(j, game.players[i]);
					break;
				}
			}
		}
	}
	_tprintf(TEXT("\n[SERVER] Ranking has been updated.\n"));
}

void upgradeLevel(void) {
	if (game.level == game.configs.max_levels) {
		// TODO WIN CODE
		game.gameActive = 0;
		game.win = 1;
		_tprintf(TEXT("\n[SERVER] Game win.\n"));
		updateRanking();
		writeRankingData(ranking);
	}
	else {
		game.level++;
		game.board.numBricks = levels[game.level - 1].num_bricks;
		for (int i = 0; i < GAME_INITIAL_NUM_BRICKS; i++) {
			game.board.bricks[i] = levels[game.level - 1].bricks[i];
		}
		game.board.ball.speed = levels[game.level - 1].ball_speed;
		resetBallPosition();
		resetPaddlesPosition();
	}
}

DWORD WINAPI ThreadBallMovement(void) {
	int speed = 1;
	while (game.gameActive) {
		WaitForSingleObject(hMutexData, INFINITE);
		moveBall(&(game.board.ball), &(game.board), game.players, game.configs.max_players);
		if (checkLost(&(game.gameActive), &(game.board.ball), game.players, game.configs.max_players, game.configs.paddle_initial_line) == SUCCESS) {
			if (game.gameActive) {
				resetBallPosition();
				resetPaddlesPosition();
			}
			else {
				// TODO LOSE CODE
				_tprintf(TEXT("\n[SERVER] Game lost.\n"));
				game.win = 0;
				updateRanking();
				writeRankingData(ranking);
			}
		}
		if (checkWin(game.board.bricks, game.board.numBricks) == SUCCESS) {
			_tprintf(TEXT("\n[SERVER] new level.\n"));
			upgradeLevel();
		}
		SendBroadcast(game);
		speed = game.board.ball.speed;
		ReleaseMutex(hMutexData);
		Sleep(100 / speed);
	}
	return SUCCESS;
}

DWORD WINAPI ThreadBricksMovement(void) {
	int direcao = rand() % 1, speed = 1, count = 0, level = 1;

	while (game.gameActive) {
		count++;
		WaitForSingleObject(hMutexData, INFINITE);
		moveBricks(game.board.bricks, game.board.numBricks, game.board, direcao);
		SendBroadcast(game);
		speed = game.board.bricks_speed;
		level = game.level;
		ReleaseMutex(hMutexData);
		Sleep(100 / speed);
		if (count == (2 * level)) {
			count = 0;
			direcao = !direcao;
		}
	}

	return SUCCESS;

}

int createThreads(void) {
	pThreadPlayerRequests = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadPlayerRequests, NULL, 0, NULL);
	if (pThreadPlayerRequests == NULL) {
		_tprintf(TEXT("\n[SERVER] pThreadPlayerRequests error %d.\n"), GetLastError());
		return FAILURE;
	}
	_tprintf(TEXT("\n[SERVER] Thread for player requests created.\n"));

	return SUCCESS;
}

int createMutexs(void) {
	hMutexData = CreateMutex(NULL, FALSE, NULL);
	if (hMutexData == NULL) {
		_tprintf(TEXT("\n[SERVER] Create mutex failed and error number %d \n"), GetLastError());
		return FAILURE;
	}
	_tprintf(TEXT("\n[SERVER] Mutex for data created.\n"));
	return SUCCESS;
}

void showLevels(void) {
	_tprintf(TEXT("\n[SERVER]\n --- LEVELS ---\n"));
	for (int i = 0; i < game.configs.max_levels; i++) {
		_tprintf(TEXT("\nLevel %d:\n"), i + 1);
		_tprintf(TEXT("Ball Speed: %d\n"), levels[i].ball_speed);
		_tprintf(TEXT("Number of bricks: %d\n"), levels[i].num_bricks);
		for (int j = 0; j < levels[i].num_bricks; j++) {
			_tprintf(TEXT("Brick %02d: Type: "), j);
			switch (levels[i].bricks[j].type) {
			case BRICK_TYPE_NORMAL:
				_tprintf(TEXT("Normal    "));
				break;
			case BRICK_TYPE_RESISTANT:
				_tprintf(TEXT("Resistant "));
				break;
			case BRICK_TYPE_MAGIC:
				_tprintf(TEXT("Magic     "));
				break;
			}
			_tprintf(TEXT("Location: (%03d,%03d) Lifes: %d Width: %d Points: %d\n"), levels[i].bricks[j].coords.x, levels[i].bricks[j].coords.y, levels[i].bricks[j].lifes, levels[i].bricks[j].width, levels[i].bricks[j].points);
		}

	}
}

void showConfigs(void) {
	_tprintf(TEXT("\n[SERVER]\n --- CONFIGS ---\n"));
	_tprintf(TEXT("Number of max players: %d\n"), game.configs.max_players);
	_tprintf(TEXT("Number of max levels: %d\n"), game.configs.max_levels);
	_tprintf(TEXT("Number of initial lives: %d\n"), game.configs.initial_lives);
	_tprintf(TEXT("Number of initial paddle width: %d\n"), game.configs.initial_paddle_width);
	_tprintf(TEXT("Number of paddle initial line: %d\n"), game.configs.paddle_initial_line);
	_tprintf(TEXT("Number of ball initial line: %d\n"), game.configs.ball_initial_line);
	_tprintf(TEXT("Number of brick normal points: %d\n"), game.configs.brick_normal_points);
	_tprintf(TEXT("Number of brick normal width: %d\n"), game.configs.brick_normal_width);
	_tprintf(TEXT("Number of brick normal max lives: %d\n"), game.configs.brick_normal_max_lives);
	_tprintf(TEXT("Number of brick resistant points: %d\n"), game.configs.brick_resistant_points);
	_tprintf(TEXT("Number of brick resistant width: %d\n"), game.configs.brick_resistant_width);
	_tprintf(TEXT("Number of brick resistant max lives: %d\n"), game.configs.brick_resistant_max_lives);
	_tprintf(TEXT("Number of brick magic points: %d\n"), game.configs.brick_magic_points);
	_tprintf(TEXT("Number of brick magic width: %d\n"), game.configs.brick_magic_width);
	_tprintf(TEXT("Number of brick magic max lives: %d\n"), game.configs.brick_magic_max_lives);
}

void prepareThingsToPlay(void) { // TODO change to configs
	WaitForSingleObject(hMutexData, INFINITE);
	for (int i = 0; i < game.configs.max_players; i++) {
		game.players[i].paddle.width = game.configs.initial_paddle_width / getNumberOfPlayersOnlineFromGame(game.players, game.configs.max_players);
		game.players[i].lives = game.configs.initial_lives;
		game.players[i].score = 0;
	}
	resetPaddlesPosition();
	game.level = 1;
	game.win = -1;
	game.board.numBricks = levels[game.level - 1].num_bricks;
	for (int i = 0; i < GAME_INITIAL_NUM_BRICKS; i++) {
		game.board.bricks[i] = levels[game.level - 1].bricks[i];
	}
	game.board.ball.speed = levels[game.level - 1].ball_speed;
	game.board.bricks_speed = levels[game.level - 1].bricks_speed;
	game.board.coords.x = 0;
	game.board.coords.y = 0;
	resetBallPosition();
	ReleaseMutex(hMutexData);
}

void startGame(void) {
	game.gameActive = 1;
	_tprintf(TEXT("\n[SERVER] Game initiated.\n"));
	prepareThingsToPlay();
	//Thread Creation
	pThreadBallMovement = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadBallMovement, NULL, 0, NULL);
	if (pThreadBallMovement == NULL) {
		_tprintf(TEXT("\n[SERVER] pThreadBallMovement error %d.\n"), GetLastError());
	}
	_tprintf(TEXT("\n[SERVER] Thread for ball movement created.\n"));

	pThreadBricksMovement = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadBricksMovement, NULL, 0, NULL);

	if (pThreadBricksMovement == NULL) {
		_tprintf(TEXT("\n[SERVER] pThreadBricksMovement error %d.\n"), GetLastError());
	}
	_tprintf(TEXT("\n[SERVER] Thread for bricks movement created.\n"));


	SendBroadcast(game);
}

void endGame(void) {
	WaitForSingleObject(hMutexData, INFINITE);
	game.gameActive = 0;
	SendBroadcast(game);
	ReleaseMutex(hMutexData);
	WaitForSingleObject(pThreadBricksMovement, INFINITE);
	WaitForSingleObject(pThreadBallMovement, INFINITE);
}

void startServiceNt(void) {
	//InstallService(
	//	SERVICE_NAME,               // Name of service 
	//	SERVICE_DISPLAY_NAME,       // Name to display 
	//	SERVICE_START_TYPE,         // Service start type 
	//	SERVICE_DEPENDENCIES,       // Dependencies 
	//	SERVICE_ACCOUNT,            // Service running account 
	//	SERVICE_PASSWORD            // Password of the account 
	//);
}

void closeServiceNt(void) {
	//UninstallService(SERVICE_NAME);
}

void readCommands(void) {
	TCHAR cmd[MAX_CMD];

	while (game.run) {
		_tprintf(TEXT("\n[SERVER] Type a commmand: "));
		_fgetts(cmd, MAX_CMD, stdin);
		cmd[_tcslen(cmd) - 1] = TEXT('\0');

		if (_tcsncmp(cmd, TEXT("start game"), 10) == 0) {
			if (getNumberOfPlayersOnlineFromGame(game.players, game.configs.max_players) > 0) {
				startGame();
			}
			else {
				_tprintf(TEXT("\n[SERVER] Can't start a game without players.\n"));
			}
		}
		else if (_tcsncmp(cmd, TEXT("end game"), 8) == 0) {
			WaitForSingleObject(hMutexData, INFINITE);
			logoutAllPlayersFromGame(game.players, game.configs.max_players);
			ReleaseMutex(hMutexData);
			endGame();
			_tprintf(TEXT("\n[SERVER] Game terminated.\n"));
		}
		else if (_tcsncmp(cmd, TEXT("show configs"), 12) == 0) {
			showConfigs();
		}
		else if (_tcsncmp(cmd, TEXT("show levels"), 11) == 0) {
			showLevels();
		}
		else if (_tcsncmp(cmd, TEXT("show ranking"), 12) == 0) {
			showTopRanking(ranking);
		}
		else if (_tcsncmp(cmd, TEXT("start servicent"), 15) == 0) {
			startServiceNt();
		}
		else if (_tcsncmp(cmd, TEXT("close servicent"), 15) == 0) {
			closeServiceNt();
		}
		else if (_tcsncmp(cmd, TEXT("shutdown"), 8) == 0) {
			_tprintf(TEXT("\n[SERVER] Server will shutdown.\n"));
			_tprintf(TEXT("\n[SERVER] Notifying all players...\n"));
			endGame();
			WaitForSingleObject(hMutexData, INFINITE);
			game.gameActive = 0;
			game.run = 0;
			SendBroadcast(game);
			logoutAllPlayersFromServer(game.players, game.configs.max_players);
			ReleaseMutex(hMutexData);
			_tprintf(TEXT("\n[SERVER] Players have been notified. Shutting down..\n"));
			WaitForSingleObject(hMutexData, INFINITE);
		}
	}
}