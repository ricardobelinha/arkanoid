#include "communication.h"

// Global Variables
GameData game;
Configurations configs;
int run, inGame;
int remote;
HANDLE hMutexData, pThreadGetGame;
HWND hWnd;

int makeLogin(TCHAR* username, TCHAR * ip) {
	Answer answer;
	answer = LoginUser(username, remote, ip); // TODO remote not working
	return answer.type;
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

DWORD WINAPI getGame(void) {
	inGame = 1;
	do {
		WaitForSingleObject(hMutexData, INFINITE);
		game = ReceiveBroadcast();
		if (!game.run) {
			if (MessageBox(hWnd, TEXT("The server disconnected. We apologize for the inconvenience."), TEXT("Server Status"), MB_OK) == IDOK) {
				PostMessage(hWnd, WM_DESTROY, NULL, NULL);
				return SUCCESS;
			}
		}

		if (inGame && !game.gameActive) {
			/*if (game.win == 1) {
				MessageBox(hWnd, TEXT("You won!"), TEXT("WIN"), MB_OK);
			}
			else {
				MessageBox(hWnd, TEXT("You lost :("), TEXT("LOSE"), MB_OK);
			}*/
			inGame = 0;
			InvalidateRect(hWnd, NULL, FALSE);
		}

		if (inGame && game.gameActive)
			InvalidateRect(hWnd, NULL, FALSE);
		ReleaseMutex(hMutexData);

	} while (game.run);

	return SUCCESS;
}

int createThreads(void) {
	pThreadGetGame = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getGame, NULL, 0, NULL);
	if (pThreadGetGame == NULL) {
		_tprintf(TEXT("\n[CLIENT] pThreadGetGame error %d.\n"), GetLastError());
		return FAILURE;
	}
	_tprintf(TEXT("\n[CLIENT] Thread for get the game has been created.\n"));
	inGame = 1;
	return SUCCESS;
}

int readAction() {
	return 0;
}

void showTopRanking(TCHAR* info) {
	TCHAR topTenEntry[256];
	Answer answer = PingServer();
	if (answer.type == ANSWER_PING_SERVER_OFFLINE) {
		_tcscpy_s(info, 4096, TEXT("\n[CLIENT] The server disconnected."));
		return;
	}

	answer = GetTopTen();

	//_tprintf(TEXT("\n\nArkanoid - Top Ranking\n\n"));
	_tcscpy_s(info, 4096, TEXT("\tRank\tUsername\t\tScore\n"));
	for (int i = 0; i < GAME_TOP_RANK; i++) {
		_stprintf_s(topTenEntry, TEXT("\t%2d\t%-35s\t%06d\n"), i + 1, answer.ranking[i].username, answer.ranking[i].score);
		_tcscat_s(info, 4096, topTenEntry);
	}
}