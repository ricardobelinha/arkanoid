#include "ranking.h"
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define REGISTRY_KEY_PATH TEXT("Games\\Arkanoid")


bool doesRegistryKeyAlreadyExists() {
	HKEY hKey;
	LONG lError = RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, NULL, KEY_ALL_ACCESS, &hKey);

	return !(lError == ERROR_FILE_NOT_FOUND);
}

HKEY OpenRegistryKey(void) {
	HKEY hKey;
	LONG lError = RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, NULL, KEY_ALL_ACCESS, &hKey);

	if (ERROR_FILE_NOT_FOUND == lError) {
		lError = RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	}

	if (lError != ERROR_SUCCESS) {
		_tprintf(TEXT("[SERVER] Couldn't open or register key.\n"));
		exit(-1);
	}

	return hKey;
}

//void deleteRegistryKey


void putValueOnRegistry(HKEY hRootKey, LPCTSTR lpVal, Ranking data) {
	LONG nErr = RegSetValueEx(hRootKey, lpVal, 0, REG_BINARY, (LPBYTE)& data, sizeof(Ranking));

	if (nErr != ERROR_SUCCESS) {
		_tprintf(TEXT("[SERVER] Couldn't insert data in registry.\n"));
	}
}

int initializeRankingInRegistry(Ranking* rank) {
	// If ranking already exists,, don't initialize
	if (doesRegistryKeyAlreadyExists()) {
		return FAILURE;
	}

	// Get Registry Key
	HKEY hKey = OpenRegistryKey();

	TCHAR msg[MAX];
	// Criar o meu ranking (vazio)
	for (int i = 0; i < GAME_TOP_RANK; i++) {
		_stprintf_s(msg, MAX, TEXT("%d"), i);
		putValueOnRegistry(hKey, msg, rank[i]);
	}

	return SUCCESS;
}

void getRankingData(Ranking* rank) {
	HKEY hKey = OpenRegistryKey();
	TCHAR msg[MAX];

	// Recuperar os valores Values
	DWORD data = 0;
	DWORD dtype = REG_BINARY;
	DWORD dSize = sizeof(Ranking);
	LONG lErr;

	for (int i = 0; i < GAME_TOP_RANK; i++) {
		_stprintf_s(msg, MAX, TEXT("%d"), i);
		lErr = RegQueryValueEx(hKey, msg, NULL, &dtype, (LPBYTE)& rank[i], &dSize);
	}
}

void writeRankingData(Ranking* rank) {
	HKEY hKey = OpenRegistryKey();
	TCHAR msg[MAX];

	for (int i = 0; i < GAME_TOP_RANK; i++) {
		_stprintf_s(msg, MAX, TEXT("%d"), i);
		putValueOnRegistry(hKey, msg, rank[i]);
	}
}

void showTopRanking(Ranking* rank) {
	_tprintf(TEXT("\n\nArkanoid - Top Ranking\n\n"));
	_tprintf(TEXT("\tRank\tUsername\t\t\t\tScore\n"));
	for (int i = 0; i < GAME_TOP_RANK; i++) {
		_tprintf(TEXT("\t%2d\t%-35s\t%06d\n"), i + 1, rank[i].username, rank[i].score);
	}
}