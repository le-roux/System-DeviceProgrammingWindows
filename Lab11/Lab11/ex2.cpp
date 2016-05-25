#include "ex2.h"

BUFFER buffer;
CRITICAL_SECTION readCS, writeCS;
DWORD P, C, N, T;

INT _tmain(INT argc, LPTSTR argv[]) {
	LPHANDLE producersHandles, consumersHandles;
	LPDWORD producersIds, consumersIds;

	if (argc != 5) {
		_ftprintf(stderr, _T("Wrong number of arguments.\nProper usage :  %s P C N T\n"), argv[0]);
		return 1;
	}

	P = _tcstod(argv[1], _T('\0'));
	C = _tcstod(argv[2], _T('\0'));
	N = _tcstod(argv[3], _T('\0'));
	T = _tcstod(argv[4], _T('\0'));

	
	producersHandles = (LPHANDLE)malloc(P * sizeof(HANDLE));
	consumersHandles = (LPHANDLE)malloc(C * sizeof(HANDLE));
	producersIds = (LPDWORD)malloc(P * sizeof(DWORD));
	consumersIds = (LPDWORD)malloc(C * sizeof(DWORD));

	InitializeCriticalSection(&readCS);
	InitializeCriticalSection(&writeCS);

	for (DWORD i = 0; i < P; i++) {
		producersHandles[i] = CreateThread(NULL, 0, producer, &T, 0, &producersIds[i]);
		if (producersHandles[i] == NULL) {
			_ftprintf(stderr, _T("Error %i when creating a producer thread\n"), GetLastError());
			return 1;
		}
	}

	for (DWORD i = 0; i < C; i++) {
		consumersHandles[i] = CreateThread(NULL, 0, consumer, &consumersIds[i], 0, &consumersIds[i]);
		if (consumersHandles[i] == NULL) {
			_ftprintf(stderr, _T("Error %i when creating a consumer thread\n"), GetLastError());
			return 1;
		}
	}

	WaitForMultipleObjects(C, consumersHandles, TRUE, INFINITE);
	free(buffer.buffer);
	free(producersHandles);
	free(consumersHandles);
	free(producersIds);
	free(consumersIds);
	DeleteCriticalSection(&readCS);
	DeleteCriticalSection(&writeCS);
	return 0;
}

DWORD WINAPI producer(LPVOID arg) {
	DWORD T, waitingTime, value;
	T = *(LPDWORD)arg;
	while (1) {
		waitingTime = rand() % T;
		Sleep(waitingTime);
		value = rand();
		writeBuffer(value);
	}
}

DWORD WINAPI consumer(LPVOID arg) {
	DWORD value, waitingTime, threadId;
	threadId = *(LPDWORD)arg;
	while (1) {
		waitingTime = rand() % T;
		Sleep(waitingTime);
		value = readBuffer();
		_ftprintf(stdout, _T("Consumer %i has read %i\n"), threadId, value);
	}
}

DWORD readBuffer() {
	DWORD value;
	EnterCriticalSection(&readCS);
	value = buffer.buffer[buffer.indexRead];
	buffer.indexRead++;
	buffer.indexRead %= N;
	LeaveCriticalSection(&readCS);
	return value;
}

VOID writeBuffer(DWORD value) {
	EnterCriticalSection(&writeCS);
	buffer.buffer[buffer.indexWrite];
	buffer.indexWrite++;
	buffer.indexWrite %= N;
	LeaveCriticalSection(&writeCS);
}

VOID initBuffer(BUFFER* buffer) {
	buffer->buffer = (LPINT)malloc(N * sizeof(INT));
	buffer->indexRead = 0;
	buffer->indexWrite = 0;
	buffer->semaphoreEmpty = CreateSemaphore(NULL, 0, N, NULL);
	buffer->semaphoreFull = CreateSemaphore(NULL, N, N, NULL);
}