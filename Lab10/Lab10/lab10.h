#ifndef LAB10_H
#define LAB10_H

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

volatile LPTSTR* entries;

/**
 * A simple structure used to pass the needed data to the reading threads.
 */
typedef struct ARGS {
	LPTSTR dirName;
	DWORD threadNb;
} ARGS;

/**
 * Used to make a barrier to force the reading threads to read only
 * one entry at a time.
 */
volatile DWORD counter;
DWORD nbThreads;
ARGS* argsList;
CRITICAL_SECTION criticalSection;
HANDLE eventReaders;
HANDLE eventComparator;

DWORD WINAPI readingThread(LPVOID arg);
VOID exploreDirectory(LPTSTR dirName, DWORD threadId, DWORD threadNb);
DWORD WINAPI compare(LPVOID arg);
static DWORD FileType(LPWIN32_FIND_DATA fileInfo);
LPTSTR addFinalSlash(LPTSTR input);

#endif
