#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

static DWORD FileType(LPWIN32_FIND_DATA fileInfo);
DWORD WINAPI readingThread(LPVOID arg);
VOID exploreDirectory(LPTSTR dirName, DWORD threadId);

INT _tmain(INT argc, LPTSTR argv[]) {
	if (argc == 1) {
		_ftprintf(stderr, _T("Wrong number of arguments\nUsage : %s + N directories"), argv[0]);
		return 1;
	}

	DWORD nbThreads;
	HANDLE* handles;
	DWORD* threadsId;
	nbThreads = argc - 1;
	handles = (HANDLE*)malloc(nbThreads * sizeof(HANDLE));
	threadsId = (DWORD*)malloc(nbThreads * sizeof(DWORD));
	for (DWORD i = 0; i < nbThreads; i++) {
		handles[i] = CreateThread(NULL, 0, readingThread, argv[i + 1], 0, &threadsId[i]);
	}
	WaitForMultipleObjects(nbThreads, handles, TRUE, INFINITE);
	TCHAR a;
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

DWORD WINAPI readingThread(LPVOID arg) {
	LPTSTR dirName;
	dirName = (LPTSTR)arg;
	exploreDirectory(dirName, GetCurrentThreadId());
	return 0;
}

VOID exploreDirectory(LPTSTR dirName, DWORD threadId) {
	HANDLE dir;
	LPTSTR pattern;
	TCHAR newDirName[500];
	WIN32_FIND_DATA fileInfo;
	DWORD ret;
	pattern = (LPTSTR)malloc(_tcslen(dirName) + sizeof(TCHAR));
	_tcscpy(pattern, dirName);
	_tcscat(pattern, _T("*"));
	dir = FindFirstFile(pattern, &fileInfo);
	if (dir == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening directory %s\n"), GetLastError(), pattern);
		return;
	}
	do {
		if (FileType(&fileInfo) == TYPE_FILE) {
			_ftprintf(stdout, _T("Thread %i : file %s\n"), threadId, fileInfo.cFileName);
		}
		else if (FileType(&fileInfo) == TYPE_DIR) {
			_tcscpy(newDirName, dirName);
			_tcscat(newDirName, fileInfo.cFileName);
			_tcscat(newDirName, _T("/"));
			exploreDirectory(newDirName, threadId);
		}
	} while (FindNextFile(dir, &fileInfo));
	ret = FindClose(dir);
	if (ret == FALSE) {
		_ftprintf(stderr, _T("Error when closing directory %s\n"), dirName);
	}
}

static DWORD FileType(LPWIN32_FIND_DATA fileInfo) {
	BOOL isDir;
	DWORD fileType;
	isDir = ((fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
	fileType = TYPE_FILE;
	if (isDir) {
		if (_tcscmp(fileInfo->cFileName, _T(".")) == 0
			|| _tcscmp(fileInfo->cFileName, _T("..")) == 0) {
			fileType = TYPE_DOT;
		}
		else
			fileType = TYPE_DIR;
	}
	return fileType;
}