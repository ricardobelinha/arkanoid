#include "stdafx.h"
#include "DLL.h"

#pragma region RequestBuffer

TCHAR bufferRequestSharedMemoryName[] = TEXT("REQUEST BUFFER MEMORY");
Request(*memoryRequestPtr)[BUFFER_SIZE];

TCHAR canWriteRequestSemaphoreName[] = TEXT("CAN WRITE REQUEST SEMAPHORE");
TCHAR canReadRequestSemaphoreName[] = TEXT("CAN READ REQUEST SEMAPHORE");

HANDLE canWriteRequest, canReadRequest;
HANDLE hIndexRequestIn, hIndexRequestOut, hMutexRequestIndexIn, hMutexRequestIndexOut;
HANDLE hRequestMemory;

int* indexRequestOut, * indexRequestIn;

#pragma endregion

#pragma region InitalizeResources

DWORD WINAPI initializeResourcesRequestBufferZone(void) {
	int initialized = 0;

	canWriteRequest = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, canWriteRequestSemaphoreName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		initialized = 1;

	canReadRequest = CreateSemaphore(NULL, 0, BUFFER_SIZE, canReadRequestSemaphoreName);
	hRequestMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Request) * BUFFER_SIZE, bufferRequestSharedMemoryName);
	hIndexRequestOut = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int), NULL);
	hIndexRequestIn = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int), NULL);
	hMutexRequestIndexOut = CreateMutex(NULL, FALSE, TEXT("Mutex Request Index Out"));
	hMutexRequestIndexIn = CreateMutex(NULL, FALSE, TEXT("Mutex Request Index In"));

	if (canWriteRequest == NULL || canReadRequest == NULL || hRequestMemory == NULL
		|| hIndexRequestOut == NULL || hIndexRequestIn == NULL || hMutexRequestIndexOut == NULL
		|| hMutexRequestIndexIn == NULL) {
		_tprintf(TEXT("\n[ERROR - DLL] Request Buffer - Windows object creation failed: (%d)\n"), GetLastError());
		return FAILURE;
	}

	memoryRequestPtr = (Request(*)[BUFFER_SIZE])MapViewOfFile(hRequestMemory, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, sizeof(Request) * BUFFER_SIZE);
	if (memoryRequestPtr == NULL) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - MapViewOfFile memoryRequestPtr error (%d)\n"), GetLastError());
		return FAILURE;
	}

	indexRequestIn = (int(*))MapViewOfFile(hIndexRequestIn, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, sizeof(int));
	if (indexRequestIn == NULL) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - MapViewOfFile indexRequestIn error (%d)\n"), GetLastError());
		return FAILURE;
	}

	indexRequestOut = (int(*))MapViewOfFile(hIndexRequestOut, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, sizeof(int));
	if (indexRequestOut == NULL) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - MapViewOfFile indexRequestOut error (%d)\n"), GetLastError());
		return FAILURE;
	}

	if (!initialized) {
		*indexRequestOut = 0;
		*indexRequestIn = 0;
	}

	return SUCCESS;
}

#pragma endregion

#pragma region CloseResources

DWORD WINAPI closeResourcesRequestBufferZone(void) {
	BOOL UnmapResult;

	UnmapResult = UnmapViewOfFile(memoryRequestPtr);
	if (UnmapResult == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - UnmapViewOfFile memoryRequestPtr error (%d)\n"), GetLastError());
		return FAILURE;
	}

	UnmapResult = UnmapViewOfFile(indexRequestIn);
	if (UnmapResult == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - UnmapViewOfFile indexRequestIn error (%d)\n"), GetLastError());
		return FAILURE;
	}

	UnmapResult = UnmapViewOfFile(indexRequestOut);
	if (UnmapResult == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - UnmapViewOfFile indexRequestIn error (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hRequestMemory);
	if (hRequestMemory == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - CloseHandle hRequestMemory (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(canWriteRequest);
	if (canWriteRequest == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - CloseHandle canWriteRequest error (%d)\n"), GetLastError());
		return -1;
	}

	CloseHandle(canReadRequest);
	if (canReadRequest == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - CloseHandle canReadRequest (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hIndexRequestOut);
	if (hIndexRequestOut == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - CloseHandle hIndexRequestOut (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hIndexRequestIn);
	if (hIndexRequestIn == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - CloseHandle hIndexRequestIn (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hMutexRequestIndexOut);
	if (hMutexRequestIndexOut == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - CloseHandle hMutexRequestIndexOut (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hMutexRequestIndexIn);
	if (hMutexRequestIndexIn == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Request Buffer - CloseHandle hMutexRequestIndexIn (%d)\n"), GetLastError());
		return FAILURE;
	}

	return SUCCESS;
}

#pragma endregion

#pragma region BufferManagement

void writeToBuffer(Request request) {
	int temp;

	WaitForSingleObject(canWriteRequest, INFINITE);
	WaitForSingleObject(hMutexRequestIndexIn, INFINITE);
	temp = *indexRequestIn;
	*indexRequestIn = (*indexRequestIn + 1) % BUFFER_SIZE;
	ReleaseMutex(hMutexRequestIndexIn);
	//(*memoryRequestPtr)[temp] = request;
	CopyMemory(&((*memoryRequestPtr)[temp]), &request, sizeof(Request)); // TODO deve haver uma forma mais simples de representar..
	ReleaseSemaphore(canReadRequest, 1, NULL);
}

Request readFromBuffer(void) {
	Request request;
	int temp;

	WaitForSingleObject(canReadRequest, INFINITE);
	WaitForSingleObject(hMutexRequestIndexOut, INFINITE);
	temp = *indexRequestOut;
	*indexRequestOut = (*indexRequestOut + 1) % BUFFER_SIZE;
	ReleaseMutex(hMutexRequestIndexOut);
	//request = (*memoryRequestPtr)[temp];
	CopyMemory(&request, &((*memoryRequestPtr)[temp]), sizeof(Request));
	ReleaseSemaphore(canWriteRequest, 1, NULL);

	return request;
}

#pragma endregion

#pragma region ClientActions

void SendRequest(TCHAR * username, int actionType) {
	Request request;
	request.type = actionType;
	_tcscpy_s(request.username, MAX_NAME, username);

	writeToBuffer(request);
}

void SendPingRequest(void) {
	Request request;
	request.type = ACTION_PING_SERVER;

	writeToBuffer(request);
}

void SendTopTenRequest(void) {
	Request request;
	request.type = ACTION_GET_TOP_10;

	writeToBuffer(request);
}

Answer Login(TCHAR * username) {
	SendRequest(username, ACTION_LOGIN);
	return ReceiveAnswer();
}

Answer JoinGame(TCHAR* username) {
	SendRequest(username, ACTION_JOIN_GAME);
	return ReceiveAnswer();
}

Answer PingServer(void) {
	SendPingRequest();
	
	if (PingAnswer() == true) {
		return ReceiveAnswer();
	}
	
	Answer answer;
	answer.type = ANSWER_PING_SERVER_OFFLINE;
	return answer;
}

Answer GetTopTen(void) {
	SendTopTenRequest();
	return ReceiveAnswer();
}

#pragma endregion

#pragma region ServerActions

Request ReceiveRequest(void) {
	return readFromBuffer();
}

#pragma endregion