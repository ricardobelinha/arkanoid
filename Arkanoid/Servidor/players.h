#pragma once

#include "../DLL/structs.h"

void logoutAllPlayersFromGame(Player * players, int max_players);
void logoutAllPlayersFromServer(Player* players, int max_players);
int logoutPlayerFromServer(Player * players, TCHAR* username, int max_players);
int logoutPlayerFromGame(Player * players, TCHAR* username, int max_players);
int loginPlayerToServer(Player * players, TCHAR* username, int max_players);
int loginPlayerToPlayGame(Player * players, TCHAR* username, int max_players);
int loginPlayerToSpectateGame(Player* players, TCHAR* username, int max_players);
int getNumberOfPlayersOnlineFromServer(Player * players, int max_players);
int getNumberOfPlayersOnlineFromGame(Player * players, int max_players);
int movePlayerPaddle(Player * players, TCHAR* username, int maxPlayers, Board board, int moveDirection);