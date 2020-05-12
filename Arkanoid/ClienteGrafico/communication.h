#pragma once
#include <tchar.h>
#include "../DLL/DLL.h"

#define DEFAULT_KEY_MOVE_RIGHT TEXT('d')
#define DEFAULT_KEY_MOVE_LEFT TEXT('a')
#define DEFAULT_KEY_QUIT 27

typedef struct {
	TCHAR username[MAX_NAME];
	TCHAR move_right;
	TCHAR move_left;
} Configurations;

int makeLogin(TCHAR* username, TCHAR * ip);
int createMutexs(void);
int createThreads(void);
void showTopRanking(TCHAR* info);

// Variáveis Globais
extern Configurations configs;
extern GameData game;
extern HWND hWnd;
extern HANDLE hMutexData;
extern int inGame;
extern int remote;