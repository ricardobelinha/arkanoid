#include "stdafx.h"
#include "DLL.h"

#include <strsafe.h>
#include <aclapi.h>
#include <sddl.h>

typedef struct {
	int valid;
	TCHAR username[MAX_NAME];
	HANDLE hPipeToClient;
} RemoteClient;

TCHAR PIPE_NAME_MESSAGES[] = TEXT("\\\\192.168.43.43\\server-dir\\pipe"); // TODO meter IP configurável
SECURITY_ATTRIBUTES sa;
RemoteClient remoteClients[GAME_MAX_PLAYERS];
int* runServer;

DWORD WINAPI initializeRemoteClients(void) {
	for (int i = 0; i < GAME_MAX_PLAYERS; i++)
	{
		remoteClients[i].valid = 0;
		remoteClients[i].hPipeToClient = INVALID_HANDLE_VALUE;
	}
	return SUCCESS;
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

DWORD WINAPI initializeSecurityAttributes() {
	/*TCHAR szSD[] = TEXT("D:")
		TEXT("(A;OICI;GA;;;BG)")
		TEXT("(A;OICI;GA;;;AN)")
		TEXT("(A;OICI;GA;;;AU)")
		TEXT("(A;OICI;GA;;;BA)");
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = FALSE;

	ConvertStringSecurityDescriptorToSecurityDescriptor(
		szSD,
		SDDL_REVISION_1,
		&(sa.lpSecurityDescriptor),
		NULL
	);

	return SUCCESS;*/

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

	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = TRUE;
	return SUCCESS;
}

int findNextAvailableRemoteClientPos(void) {
	int i;
	for (i = 0; i < GAME_MAX_PLAYERS && remoteClients[i].valid; i++)
		;
	if (i == GAME_MAX_PLAYERS)
		return -1; // No positions available
	return i;
}

int findRemoteClientPosByUsername(TCHAR* name) {
	int i;
	for (i = 0; i < GAME_MAX_PLAYERS; i++)
		if (remoteClients[i].valid && _tcsncmp(remoteClients[i].username, name, MAX_NAME) == 0)
			return i;

	return -1; // Player not found
}

DWORD WINAPI fakeConnection(HANDLE* hPipe) {
	TCHAR path[] = TEXT("C:\\server-dir\\pipemessages");
	if (!WaitNamedPipe(path, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERROR] Wait pipe '%s'! (WaitNamedPipe)\n"), path);
		return FAILURE;
	}
	//_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
	*hPipe = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);
	if (*hPipe == NULL) {
		_tprintf(TEXT("[ERROR] Ligar ao pipe '%s'! (CreateFile)\n"), path);
		return FAILURE;
	}
	return SUCCESS;
}

DWORD WINAPI connectFakeClient(void) {
	HANDLE hPipe;
	if (fakeConnection(&hPipe) == SUCCESS) {
		CloseHandle(hPipe);
		return SUCCESS;
	}
	return FAILURE;
}


DWORD WINAPI initializeClientPipeConnection(HANDLE* hPipe, TCHAR * ip) {
	TCHAR path[100];
	_stprintf_s(path, 100, TEXT("\\\\%s\\server-dir\\pipe"), ip);
	if (!WaitNamedPipe(path, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERROR] Wait pipe '%s'! (WaitNamedPipe)\n"), path);
		return FAILURE;
	}
	//_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
	*hPipe = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);
	if (*hPipe == NULL) {
		_tprintf(TEXT("[ERROR] Ligar ao pipe '%s'! (CreateFile)\n"), path);
		return FAILURE;
	}
	return SUCCESS;
}

Request readRequestFromPipe(HANDLE hPipe) {
	HANDLE IOReady;
	OVERLAPPED Ov;
	BOOL ret;
	DWORD nBytes;
	Request request;

	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (IOReady == NULL) {
		_tprintf(TEXT("Event creation error...\n"));
		// TODO this should be fine
	}

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	ReadFile(hPipe, &request, sizeof(Request), &nBytes, &Ov);
	WaitForSingleObject(IOReady, INFINITE);
	ret = GetOverlappedResult(hPipe, &Ov, &nBytes, FALSE);

	if (!ret || !nBytes) {
		_tprintf(TEXT("[PIPE] %d %d... (ReadFile)\n"), ret, nBytes);
	}

	return request;
}

Answer readAnswerFromPipe(HANDLE hPipe) {
	HANDLE IOReady;
	OVERLAPPED Ov;
	BOOL ret;
	DWORD nBytes;
	Answer answer;

	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (IOReady == NULL) {
		_tprintf(TEXT("Event creation error...\n"));
		// TODO this should be fine
	}

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	ReadFile(hPipe, &answer, sizeof(Answer), &nBytes, &Ov);
	WaitForSingleObject(IOReady, INFINITE);
	ret = GetOverlappedResult(hPipe, &Ov, &nBytes, FALSE);

	if (!ret || !nBytes) {
		_tprintf(TEXT("[PIPE] %d %d... (ReadFile)\n"), ret, nBytes);
	}

	return answer;
}

GameData readGameDataFromPipe(HANDLE hPipe) {
	HANDLE IOReady;
	OVERLAPPED Ov;
	BOOL ret;
	DWORD nBytes;
	GameData gameData;

	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (IOReady == NULL) {
		_tprintf(TEXT("Event creation error...\n"));
		// TODO this should be fine
	}

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	ReadFile(hPipe, &gameData, sizeof(GameData), &nBytes, &Ov);
	WaitForSingleObject(IOReady, INFINITE);
	ret = GetOverlappedResult(hPipe, &Ov, &nBytes, FALSE);

	if (!ret || !nBytes) {
		_tprintf(TEXT("[PIPE] %d %d... (ReadFile)\n"), ret, nBytes);
	}

	return gameData;
}

DWORD WINAPI writeAnswerToPipe(HANDLE hPipe, Answer answer) {
	HANDLE IOReady;
	OVERLAPPED Ov;
	BOOL ret;
	DWORD nBytes;

	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (IOReady == NULL) {
		_tprintf(TEXT("Event creation error...\n"));
		return FAILURE;
	}

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	WriteFile(hPipe, &answer, sizeof(Answer), &nBytes, &Ov);
	WaitForSingleObject(IOReady, INFINITE);
	ret = GetOverlappedResult(hPipe, &Ov, &nBytes, FALSE);

	if (!ret || !nBytes) {
		_tprintf(TEXT("[PIPE] %d %d... (WriteFile)\n"), ret, nBytes);
		return FAILURE;
	}

	return SUCCESS;
}

DWORD WINAPI writeRequestToPipe(HANDLE hPipe, Request request) {
	HANDLE IOReady;
	OVERLAPPED Ov;
	BOOL ret;
	DWORD nBytes;

	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (IOReady == NULL) {
		_tprintf(TEXT("Event creation error...\n"));
		return FAILURE;
	}

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	WriteFile(hPipe, &request, sizeof(Request), &nBytes, &Ov);
	WaitForSingleObject(IOReady, INFINITE);
	ret = GetOverlappedResult(hPipe, &Ov, &nBytes, FALSE);

	if (!ret || !nBytes) {
		_tprintf(TEXT("[PIPE] %d %d... (WriteFile)\n"), ret, nBytes);
		return FAILURE;
	}

	return SUCCESS;
}

DWORD WINAPI writeGameDataToPipe(HANDLE hPipe, GameData gameData) {
	HANDLE IOReady;
	OVERLAPPED Ov;
	BOOL ret;
	DWORD nBytes;

	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (IOReady == NULL) {
		_tprintf(TEXT("Event creation error...\n"));
		return FAILURE;
	}

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	WriteFile(hPipe, &gameData, sizeof(GameData), &nBytes, &Ov);
	WaitForSingleObject(IOReady, INFINITE);
	ret = GetOverlappedResult(hPipe, &Ov, &nBytes, FALSE);

	if (!ret || !nBytes) {
		_tprintf(TEXT("[PIPE] %d %d... (WriteFile)\n"), ret, nBytes);
		return FAILURE;
	}

	return SUCCESS;
}

DWORD WINAPI AnswerRequestsFromClient(LPVOID param) {
	RemoteClient* client = (RemoteClient*)param;
	Request request;
	Answer answer;

	while (*runServer) {
		request = readRequestFromPipe(client->hPipeToClient);
		switch (request.type) {
		case ACTION_LOGIN:
			answer = Login(request.username);
			break;
		case ACTION_JOIN_GAME:
			answer = JoinGame(request.username);
			break;
		case ACTION_QUIT_GAME:
		case ACTION_MOVE_LEFT:
		case ACTION_MOVE_RIGHT:
			SendRequest(request.username, request.type);
			continue;
		case ACTION_SHUTDOWN: // Cliente decidiu sair, fecha a thread dele
			SendRequest(request.username, request.type);
			client->valid = 0;
			client->hPipeToClient = INVALID_HANDLE_VALUE;
			return SUCCESS;
		case ACTION_GET_TOP_10:
			answer = GetTopTen();
			break;
		case ACTION_PING_SERVER:
			answer = PingServer();
			break;
		}

		if (writeAnswerToPipe(client->hPipeToClient, answer) == SUCCESS) {
			_tprintf(TEXT("Write successful!"));
		}
	}

	return SUCCESS;
}

DWORD WINAPI waitForRemoteClients(int* run) {
	HANDLE hPipeMessages, clientThread;
	runServer = run;

	while (*runServer) {
		hPipeMessages = CreateNamedPipe(TEXT("C:\\server-dir\\pipemessages"), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT |
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, GAME_MAX_PLAYERS, sizeof(GameData), sizeof(GameData), 1000, &sa); // tamanho da maior estrutura
		if (hPipeMessages == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("GetLastError() = %d ... (CreateNamedPipe)\n"), GetLastError());
			return FAILURE;
		}
		else {
			if (!ConnectNamedPipe(hPipeMessages, NULL)) { //operação bloqueante
				_tprintf(TEXT("GetLastError() = %d ... (ConnectNamedPipe)\n"), GetLastError());
			}

			int index = findNextAvailableRemoteClientPos();
			if (index > -1) {
				remoteClients[index].valid = 1;
				remoteClients[index].hPipeToClient = hPipeMessages;
				clientThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnswerRequestsFromClient, (LPVOID) & (remoteClients[index]), 0, NULL);
			}
			else {
				_tprintf(TEXT("No more space!\n"));
			}
		}
	}

	return SUCCESS;
}

DWORD WINAPI closePipeResourcesOfClient(void) {
	//return SUCCESS;
	return FAILURE;
}

DWORD WINAPI closeServerNamedPipes(void) {

	//return SUCCESS;
	return FAILURE;
}