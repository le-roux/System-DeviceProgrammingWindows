#ifndef EX2_H
#define EX2_H

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

DWORD WINAPI producer(LPVOID arg);
DWORD WINAPI consumer(LPVOID arg);

typedef struct BUFFER {
	LPINT buffer;
	DWORD indexRead;
	DWORD indexWrite;
	HANDLE semaphoreFull;
	HANDLE semaphoreEmpty;
} BUFFER;

#endif //EX2_H