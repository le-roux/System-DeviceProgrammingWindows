#ifndef LAB13_H
#define LAB13_H

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define MAX_LENGTH 128

typedef struct Record {
	TCHAR directoryName[MAX_LENGTH];
	TCHAR inputFileName[MAX_LENGTH];
	TCHAR outputName[MAX_LENGTH];
} Record;

typedef struct OutputRecord {
	TCHAR fileName[MAX_LENGTH];
	DWORD charCount;
	DWORD lineCount;
	DWORD fileCount;
} OutputRecord;

typedef struct OUTPUT_FILE {
	LPTSTR fileName;
	CRITICAL_SECTION cs;
} OUTPUT_FILE;

DWORD WINAPI threadFunction(LPVOID arg);
DWORD WINAPI updateFunction(LPVOID arg);

DWORD GetFileType(LPWIN32_FIND_DATA fileInfo);
#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

#endif