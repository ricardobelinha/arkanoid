#include "stdafx.h"
#include "DLL.h"

#pragma region GameDataZone

TCHAR gameSharedMemoryName[] = TEXT("GAME MEMORY");
GameData* ptrGame;
HANDLE hEventGameUpdate;
HANDLE hGameData;
HANDLE hMutexGame;

#pragma endregion

#pragma region InitalizeResources

DWORD WINAPI initializeResourcesGameDataZone(void) {

#pragma region CreateResources

	hGameData = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameData), gameSharedMemoryName);

	if (hGameData == NULL) {
		_tprintf(TEXT("[ERROR] GameData - Windows object creation failed: (%d)\n"), GetLastError());
		return FAILURE;
	}

	hEventGameUpdate = CreateEvent(NULL, TRUE, FALSE, TEXT("Event Game Update"));
	if (hEventGameUpdate == NULL) {
		_tprintf(TEXT("[ERROR] GameData - Event not created...\n"));
		return FAILURE;
	}

	hMutexGame = CreateMutex(NULL, FALSE, TEXT("Mutex Game"));
	if (hMutexGame == NULL) {
		_tprintf(TEXT("[ERROR] GameData - Create mutex failed and error number %d \n"), GetLastError());
		return FAILURE;
	}

	ptrGame = (GameData*)MapViewOfFile(hGameData, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, sizeof(GameData));
	if (ptrGame == NULL) {
		_tprintf(TEXT("[ERROR] GameData - Shared Memory Mapping: (%d)\n"), GetLastError());
		return FAILURE;
	}

#pragma endregion

	return SUCCESS;
}

#pragma endregion

#pragma region CloseResources

DWORD WINAPI closeResourcesGameDataZone(void) {
	BOOL UnmapResult;

	CloseHandle(hGameData);
	if (hGameData == FALSE) {
		_tprintf(TEXT("[ERROR] GameData - CloseHandle hGameData (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hEventGameUpdate);
	if (hEventGameUpdate == FALSE) {
		_tprintf(TEXT("[ERROR] GameData - CloseHandle hEventGameUpdate (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hMutexGame);
	if (hMutexGame == FALSE) {
		_tprintf(TEXT("[ERROR] GameData - CloseHandle hMutexGame (%d)\n"), GetLastError());
		return FAILURE;
	}

	UnmapResult = UnmapViewOfFile(ptrGame);
	if (UnmapResult == FALSE) {
		_tprintf(TEXT("[ERROR] GameData - UnmapViewOfFile ptrGame error (%d)\n"), GetLastError());
		return FAILURE;
	}

	return SUCCESS;
}

#pragma endregion

#pragma region GameDataManagement

DWORD WINAPI writeGameData(GameData game) {
	WaitForSingleObject(hMutexGame, INFINITE);
	//*ptrGame = game;
	CopyMemory(ptrGame, &game, sizeof(GameData));
	SetEvent(hEventGameUpdate);
	ReleaseMutex(hMutexGame);
	ResetEvent(hEventGameUpdate);
	return SUCCESS;
}

GameData readGameData(void) {
	GameData game;

	WaitForSingleObject(hEventGameUpdate, INFINITE);
	WaitForSingleObject(hMutexGame, INFINITE);
	//game = *ptrGame;
	CopyMemory(&game, ptrGame, sizeof(GameData));
	ReleaseMutex(hMutexGame);

	return game;
}

#pragma endregion

#pragma region ServerActions

void SendBroadcast(GameData game) {
	writeGameData(game);
}

#pragma endregion

#pragma region CientActions

GameData ReceiveBroadcast(void) {
	return readGameData();
}

void WaitForGameToBegin(void) {
	WaitForSingleObject(hEventGameUpdate, INFINITE);
}

#pragma endregion