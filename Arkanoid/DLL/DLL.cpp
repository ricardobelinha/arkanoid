#include "stdafx.h"
#include "DLL.h"

HANDLE hPipeMessageClient;

DWORD WINAPI initializeResourcesDLL(void) {
	if (initializeResourcesRequestBufferZone() == SUCCESS
		&& initializeResourcesAnswerBufferZone() == SUCCESS
		&& initializeResourcesGameDataZone() == SUCCESS)
		return SUCCESS;
	return FAILURE;
}

DWORD WINAPI closeResourcesDLL(void) {
	if (closeResourcesRequestBufferZone() == SUCCESS
		&& closeResourcesAnswerBufferZone() == SUCCESS
		&& closeResourcesGameDataZone() == SUCCESS)
		return SUCCESS;
	return FAILURE;
}

DWORD WINAPI initializeRemoteCommunication(void) {
	if (initializeRemoteClients() == SUCCESS
		&& initializeSecurityAttributes() == SUCCESS)
		return SUCCESS;
	return FAILURE;
}

Answer LoginUser(TCHAR* username, int isRemote, TCHAR* ip) {
	Request request;
	Answer answer;

	if (isRemote) {
		if (initializeClientPipeConnection(&hPipeMessageClient, ip) == FAILURE) {
			answer.type = ANSWER_REMOTE_COMMUNICATION_FAILURE;
		}
		else {
			request.type = ACTION_LOGIN;
			_tcscpy_s(request.username, MAX_NAME, username);
			if (writeRequestToPipe(hPipeMessageClient, request) == FAILURE) {
				answer.type = ANSWER_REMOTE_COMMUNICATION_FAILURE;
			}
			else {
				answer = readAnswerFromPipe(hPipeMessageClient);
			}
		}
	}
	else {
		answer = Login(username);
	}

	return answer;
}