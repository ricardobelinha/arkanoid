#include "players.h"
#include "algorithms.h"

void logoutAllPlayersFromGame(Player* players, int max_players) {
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid) {
			players[i].playing = 0;
		}
	}
}

void logoutAllPlayersFromServer(Player* players, int max_players) {
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid) {
			players[i].valid = 0;
		}
	}
}

int logoutPlayerFromServer(Player* players, TCHAR* username, int max_players) {
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid && _tcscmp(username, players[i].username) == 0) {
			players[i].valid = 0;
			return SUCCESS;
		}
	}
	return FAILURE;
}

int logoutPlayerFromGame(Player* players, TCHAR* username, int max_players) {
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid && _tcscmp(username, players[i].username) == 0) {
			players[i].playing = 0;
			players[i].spectating = 0;
			return SUCCESS;
		}
	}
	return FAILURE;
}

bool playerExists(Player* players, TCHAR* username, int max_players) {
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid && _tcsncmp(players[i].username, username, MAX_NAME) == 0) {
			return true;
		}
	}
	return false;
}

int loginPlayerToServer(Player* players, TCHAR* username, int max_players) {
	for (int i = 0; i < max_players; i++) {
		if (!players[i].valid && !playerExists(players, username, max_players)) {
			_tcscpy_s(players[i].username, MAX_NAME, username);
			players[i].valid = 1;
			return SUCCESS;
		}
	}
	return FAILURE;
}

int loginPlayerToPlayGame(Player* players, TCHAR* username, int max_players) {
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid && _tcscmp(username, players[i].username) == 0) {
			players[i].playing = 1;
			return SUCCESS;
		}
	}
	return FAILURE;
}

int loginPlayerToSpectateGame(Player* players, TCHAR* username, int max_players) {
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid && _tcscmp(username, players[i].username) == 0) {
			players[i].spectating = 1;
			return SUCCESS;
		}
	}
	return FAILURE;
}

int getNumberOfPlayersOnlineFromServer(Player* players, int max_players) {
	int count = 0;
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid) count++;
	}
	return count;
}

int getNumberOfPlayersOnlineFromGame(Player* players, int max_players) {
	int count = 0;
	for (int i = 0; i < max_players; i++) {
		if (players[i].valid && players[i].playing) count++;
	}
	return count;
}

int movePlayerPaddle(Player* players, TCHAR* username, int maxPlayers, Board board, int moveDirection) {
	for (int i = 0; i < maxPlayers; i++) {
		if (players[i].valid && players[i].playing && _tcscmp(username, players[i].username) == 0) {
			return movePaddle(&players[i].paddle, board, moveDirection);
		}
	}
	return FAILURE;
}