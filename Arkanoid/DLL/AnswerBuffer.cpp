#include "stdafx.h"
#include "DLL.h"

#pragma region AnswerBuffer

TCHAR bufferAnswerSharedMemoryName[] = TEXT("ANSWER BUFFER MEMORY");
Answer(*memoryAnswerPtr)[BUFFER_SIZE];

TCHAR canWriteAnswerSemaphoreName[] = TEXT("CAN WRITE ANSWER SEMAPHORE");
TCHAR canReadAnswerSemaphoreName[] = TEXT("CAN READ ANSWER SEMAPHORE");

HANDLE canWriteAnswer, canReadAnswer;
HANDLE hIndexAnswerIn, hIndexAnswerOut, hMutexAnswerIndexIn, hMutexAnswerIndexOut;
HANDLE hAnswerMemory;

HANDLE hAnswerEvent;

int* indexAnswerOut, * indexAnswerIn;

#pragma endregion

#pragma region InitalizeResources

DWORD WINAPI initializeResourcesAnswerBufferZone(void) {
	int initialized = 0;

	canWriteAnswer = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, canWriteAnswerSemaphoreName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		initialized = 1;

	canReadAnswer = CreateSemaphore(NULL, 0, BUFFER_SIZE, canReadAnswerSemaphoreName);
	hAnswerMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Answer) * BUFFER_SIZE, bufferAnswerSharedMemoryName);
	hIndexAnswerOut = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int), NULL);
	hIndexAnswerIn = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int), NULL);
	hMutexAnswerIndexOut = CreateMutex(NULL, FALSE, TEXT("Mutex Answer Index Out"));
	hMutexAnswerIndexIn = CreateMutex(NULL, FALSE, TEXT("Mutex Answer Index In"));
	hAnswerEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("Event Answer Update"));

	if (canWriteAnswer == NULL || canReadAnswer == NULL || hAnswerMemory == NULL
		|| hIndexAnswerOut == NULL || hIndexAnswerIn == NULL || hMutexAnswerIndexOut == NULL
		|| hMutexAnswerIndexIn == NULL || hAnswerEvent == NULL) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - Windows object creation failed: (%d)\n"), GetLastError());
		return FAILURE;
	}

	memoryAnswerPtr = (Answer(*)[BUFFER_SIZE])MapViewOfFile(hAnswerMemory, FILE_MAP_WRITE, 0, 0, sizeof(Answer) * BUFFER_SIZE);
	if (memoryAnswerPtr == NULL) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - Shared Memory Mapping: (%d)\n"), GetLastError());
		return FAILURE;
	}

	indexAnswerIn = (int(*))MapViewOfFile(hIndexAnswerIn, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, sizeof(int));
	if (indexAnswerIn == NULL) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - MapViewOfFile indexAnswerIn error (%d)\n"), GetLastError());
		return FAILURE;
	}

	indexAnswerOut = (int(*))MapViewOfFile(hIndexAnswerOut, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, sizeof(int));
	if (indexAnswerOut == NULL) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - MapViewOfFile indexAnswerOut error (%d)\n"), GetLastError());
		return FAILURE;
	}

	if (!initialized) {
		*indexAnswerOut = 0;
		*indexAnswerIn = 0;
	}

	return SUCCESS;
}

#pragma endregion

#pragma region CloseResources

DWORD WINAPI closeResourcesAnswerBufferZone(void) {
	BOOL UnmapResult;

	CloseHandle(canWriteAnswer);
	if (canWriteAnswer == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - CloseHandle canWriteAnswer error (%d)\n"), GetLastError());
		return -1;
	}

	CloseHandle(canReadAnswer);
	if (canReadAnswer == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - CloseHandle canReadAnswer (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hAnswerEvent);
	if (hAnswerEvent == FALSE) {
		_tprintf(TEXT("[ERROR] GameData - CloseHandle hAnswerEvent (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hAnswerMemory);
	if (hAnswerMemory == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - CloseHandle hAnswerMemory (%d)\n"), GetLastError());
		return FAILURE;
	}

	UnmapResult = UnmapViewOfFile(memoryAnswerPtr);
	if (UnmapResult == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - UnmapViewOfFile memoryAnswerPtr error (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hIndexAnswerOut);
	if (hIndexAnswerOut == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - CloseHandle hIndexAnswerOut (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hIndexAnswerIn);
	if (hIndexAnswerIn == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - CloseHandle hIndexAnswerIn (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hMutexAnswerIndexOut);
	if (hMutexAnswerIndexOut == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - CloseHandle hMutexAnswerIndexOut (%d)\n"), GetLastError());
		return FAILURE;
	}

	CloseHandle(hMutexAnswerIndexIn);
	if (hMutexAnswerIndexIn == FALSE) {
		_tprintf(TEXT("\n[ERROR-DLL] Answer Buffer - CloseHandle hMutexAnswerIndexIn (%d)\n"), GetLastError());
		return FAILURE;
	}

	return SUCCESS;
}

#pragma endregion

#pragma region BufferManagement

void writeToBuffer(Answer answer) {
	int temp;

	WaitForSingleObject(canWriteAnswer, INFINITE);
	WaitForSingleObject(hMutexAnswerIndexIn, INFINITE);
	temp = *indexAnswerIn;
	*indexAnswerIn = (*indexAnswerIn + 1) % BUFFER_SIZE;
	ReleaseMutex(hMutexAnswerIndexIn);
	//(*memoryAnswerPtr)[temp] = answer;
	CopyMemory(&((*memoryAnswerPtr)[temp]), &answer, sizeof(Answer)); // TODO deve haver uma forma mais simples de representar..
	ReleaseSemaphore(canReadAnswer, 1, NULL);
	SetEvent(hAnswerEvent);
	ResetEvent(hAnswerEvent);
}

Answer readFromBuffer(void) {
	Answer answer;
	int temp;

	WaitForSingleObject(canReadAnswer, INFINITE);
	WaitForSingleObject(hMutexAnswerIndexOut, INFINITE);
	temp = *indexAnswerOut;
	*indexAnswerOut = (*indexAnswerOut + 1) % BUFFER_SIZE;
	ReleaseMutex(hMutexAnswerIndexOut);
	//answer = (*memoryAnswerPtr)[temp];
	CopyMemory(&answer, &((*memoryAnswerPtr)[temp]), sizeof(Answer));
	ReleaseSemaphore(canWriteAnswer, 1, NULL);

	return answer;
}

#pragma endregion

#pragma region ServerActions
void SendTop10(Ranking * ranking) {
	Answer answer;
	answer.type = ANSWER_GET_TOP_10;

	for (int i = 0; i < GAME_TOP_RANK; i++) {
		answer.ranking[i] = ranking[i];
		_tprintf(TEXT("%s - %s\n"), answer.ranking[i].username, ranking[i].username);
	}

	writeToBuffer(answer);
}

void SendAnswer(int actionType) {
	Answer answer;
	answer.type = actionType;

	writeToBuffer(answer);
}

#pragma endregion

#pragma region ClientActions

Answer ReceiveAnswer(void) {
	return readFromBuffer();
}

bool PingAnswer(void) {
	DWORD dWaitResult = WaitForSingleObject(hAnswerEvent, 3000);
	return dWaitResult == WAIT_OBJECT_0;
}

#pragma endregion