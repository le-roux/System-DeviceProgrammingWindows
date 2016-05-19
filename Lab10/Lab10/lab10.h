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
 *		dirName : the path to the directory to explore
 *		threadNb : the nb of the thread (used to access the proper entry in the "entries" array
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

/**
 * The function executed by the threads that have to read the content of the directories.
 */
DWORD WINAPI readingThread(LPVOID arg);

/**
 * The function that really explore the directories
 */
VOID exploreDirectory(LPTSTR dirName, DWORD threadId, DWORD threadNb);

/**
 * The function that compare the entries found by the readingThreads. 
 */
DWORD WINAPI compare(LPVOID arg);

/**
 * A function that returns the type of the directory entry.
 * Possible return values:
 *		-TYPE_FILE
 *		-TYPE_DIR
 *		-TYPE_DOT
 */
static DWORD FileType(LPWIN32_FIND_DATA fileInfo);

/**
 * This function adds a slash add the end of the string if there is no
 * and returns the (updated) string.
 */
LPTSTR addFinalSlash(LPTSTR input);

#endif
