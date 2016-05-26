#ifndef EX2_H
#define EX2_H

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

typedef struct BUFFER {
	LPINT buffer;
	DWORD indexRead;
	DWORD indexWrite;
	HANDLE semaphoreFull;
	HANDLE semaphoreEmpty;
} BUFFER;

DWORD WINAPI producer(LPVOID arg);
DWORD WINAPI consumer(LPVOID arg);
VOID writeBuffer(INT value, DWORD threadId);
INT readBuffer(DWORD threadId);
VOID initBuffer(BUFFER* buffer);

#endif //EX2_H