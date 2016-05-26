#include "ex2.h"

BUFFER buffer;
CRITICAL_SECTION readCS, writeCS;
DWORD N, T;

INT _tmain(INT argc, LPTSTR argv[]) {
	LPHANDLE producersHandles, consumersHandles;
	LPDWORD producersIds, consumersIds;
	DWORD P, C;

	if (argc != 5) {
		_ftprintf(stderr, _T("Wrong number of arguments.\nProper usage :  %s P C N T\n"), argv[0]);
		return 1;
	}

	P = _tcstoul(argv[1], _T('\0'), 10);
	C = _tcstoul(argv[2], _T('\0'), 10);
	N = _tcstoul(argv[3], _T('\0'), 10);
	T = _tcstoul(argv[4], _T('\0'), 10);

	initBuffer(&buffer);
	producersHandles = (LPHANDLE)malloc(P * sizeof(HANDLE));
	consumersHandles = (LPHANDLE)malloc(C * sizeof(HANDLE));
	producersIds = (LPDWORD)malloc(P * sizeof(DWORD));
	consumersIds = (LPDWORD)malloc(C * sizeof(DWORD));

	InitializeCriticalSection(&readCS);
	InitializeCriticalSection(&writeCS);
	__try {
		//Create the producer threads
		for (DWORD i = 0; i < P; i++) {
			producersHandles[i] = CreateThread(NULL, 0, producer, &producersIds[i], 0, &producersIds[i]);
			if (producersHandles[i] == NULL) {
				_ftprintf(stderr, _T("Error %i when creating a producer thread\n"), GetLastError());
				return 1;
			}
		}

		//Create the reader threads
		for (DWORD i = 0; i < C; i++) {
			consumersHandles[i] = CreateThread(NULL, 0, consumer, &consumersIds[i], 0, &consumersIds[i]);
			if (consumersHandles[i] == NULL) {
				_ftprintf(stderr, _T("Error %i when creating a consumer thread\n"), GetLastError());
				return 1;
			}
		}

		WaitForMultipleObjects(C, consumersHandles, TRUE, INFINITE);
	}
	__finally {
		free(buffer.buffer);
		buffer.buffer = NULL;
		free(producersHandles);
		producersHandles = NULL;
		free(consumersHandles);
		consumersHandles = NULL;
		free(producersIds);
		producersIds = NULL;
		free(consumersIds);
		consumersIds = NULL;
		DeleteCriticalSection(&readCS);
		DeleteCriticalSection(&writeCS);
	}
	return 0;
}

DWORD WINAPI producer(LPVOID arg) {
	DWORD threadId, waitingTime, value;
	threadId = *(LPDWORD)arg;
	srand(threadId);
	while (1) {
		waitingTime = rand() % T;
		Sleep(waitingTime);
		value = rand();
		writeBuffer(value, threadId);
	}
}

DWORD WINAPI consumer(LPVOID arg) {
	DWORD value, waitingTime, threadId;
	threadId = *(LPDWORD)arg;
	srand(threadId);
	while (1) {
		waitingTime = rand() % T;
		Sleep(waitingTime);
		value = readBuffer(threadId);
	}
}

INT readBuffer(DWORD threadId) {
	INT value;
	WaitForSingleObject(buffer.semaphoreEmpty, INFINITE);
	EnterCriticalSection(&readCS);
	value = buffer.buffer[buffer.indexRead];
	buffer.indexRead++;
	buffer.indexRead %= N;
	_ftprintf(stdout, _T("Consumer %i has read %i\n"), threadId, value);
	LeaveCriticalSection(&readCS);
	ReleaseSemaphore(buffer.semaphoreFull, 1, NULL);
	return value;
}

VOID writeBuffer(INT value, DWORD threadId) {
	WaitForSingleObject(buffer.semaphoreFull, INFINITE);
	EnterCriticalSection(&writeCS);
	buffer.buffer[buffer.indexWrite] = value;
	buffer.indexWrite++;
	buffer.indexWrite %= N;
	_ftprintf(stdout, _T("Producer %i has written %i\n"), threadId, value);
	LeaveCriticalSection(&writeCS);
	ReleaseSemaphore(buffer.semaphoreEmpty, 1, NULL);
}

VOID initBuffer(BUFFER* buffer) {
	buffer->buffer = (LPINT)malloc(N * sizeof(INT));
	buffer->indexRead = 0;
	buffer->indexWrite = 0;
	buffer->semaphoreEmpty = CreateSemaphore(NULL, 0, N, NULL);
	buffer->semaphoreFull = CreateSemaphore(NULL, N, N, NULL);
}