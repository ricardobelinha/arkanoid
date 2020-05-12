#pragma once
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "structs.h"

#define BUFFER_SIZE 10

#ifdef DLL_EXTRA_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

DWORD WINAPI initializeResourcesDLL(void);
DWORD WINAPI closeResourcesDLL(void);

DWORD WINAPI initializeResourcesRequestBufferZone(void);
DWORD WINAPI initializeResourcesAnswerBufferZone(void);
DWORD WINAPI initializeResourcesGameDataZone(void);
DWORD WINAPI closeResourcesRequestBufferZone(void);
DWORD WINAPI closeResourcesAnswerBufferZone(void);
DWORD WINAPI closeResourcesGameDataZone(void);

DWORD WINAPI initializeRemoteClients(void);
DWORD WINAPI initializeSecurityAttributes();

bool PingAnswer(void);
DWORD WINAPI initializeClientPipeConnection(HANDLE* hPipe, TCHAR * ip);
DWORD WINAPI writeRequestToPipe(HANDLE hPipe, Request request);
DWORD WINAPI writeGameDataToPipe(HANDLE hPipe, GameData gameData);
GameData readGameDataFromPipe(HANDLE hPipe);
Answer readAnswerFromPipe(HANDLE hPipe);

extern "C"
{
	DLL_IMP_API DWORD WINAPI initializeRemoteCommunication(void);

	DLL_IMP_API Answer Login(TCHAR* username);
	DLL_IMP_API Answer LoginUser(TCHAR* username, int isRemote, TCHAR* ip);
	DLL_IMP_API Answer JoinGame(TCHAR* username);
	DLL_IMP_API Answer PingServer(void);
	DLL_IMP_API Answer GetTopTen(void);
	DLL_IMP_API void SendRequest(TCHAR* username, int actionType);

	DLL_IMP_API void SendBroadcast(GameData game);
	DLL_IMP_API void WaitForGameToBegin(void);
	DLL_IMP_API GameData ReceiveBroadcast(void);

	DLL_IMP_API Request ReceiveRequest(void);
	DLL_IMP_API DWORD WINAPI waitForRemoteClients(int* run);
	DLL_IMP_API DWORD WINAPI connectFakeClient(void);

	DLL_IMP_API void SendAnswer(int actionType);
	DLL_IMP_API void SendTop10(Ranking* ranking);
	DLL_IMP_API Answer ReceiveAnswer(void);
}