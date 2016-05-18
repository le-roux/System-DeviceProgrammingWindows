#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

volatile LPTSTR* entries;


typedef struct ARGS {
	LPTSTR dirName;
	DWORD threadNb;
} ARGS;

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

INT _tmain(INT argc, LPTSTR argv[]) {
	if (argc < 3) {
		_ftprintf(stderr, _T("Wrong number of arguments\nUsage : %s + N directories"), argv[0]);
		return 1;
	}

	HANDLE* handles;
	HANDLE comparator;
	DWORD* threadsId;
	DWORD comparatorId, result;
	TCHAR a;
	counter = 0;
	nbThreads = argc - 1;
	handles = (HANDLE*)malloc(nbThreads * sizeof(HANDLE));
	threadsId = (DWORD*)malloc(nbThreads * sizeof(DWORD));
	entries = (LPTSTR*)malloc(nbThreads * sizeof(LPTSTR));
	argsList = (ARGS*)malloc(nbThreads * sizeof(ARGS));
	InitializeCriticalSection(&criticalSection);
	eventReaders = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (eventReaders == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when creating the event\n"));
		return 1;
	}
	eventComparator = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (eventComparator == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when creating the event\n"));
		return 1;
	}
	comparator = CreateThread(NULL, 0, compare, NULL, 0, &comparatorId);
	if (comparator == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when creating comparator thread\n"), GetLastError());
		return 1;
	}
	for (DWORD i = 0; i < nbThreads; i++) {
		argsList[i].dirName = argv[i + 1];
		argsList[i].threadNb = i;
		handles[i] = CreateThread(NULL, 0, readingThread, &argsList[i], 0, &threadsId[i]);
	}
	WaitForSingleObject(comparator, INFINITE);
	GetExitCodeThread(comparator, &result);
	if (result == 0) {
		_ftprintf(stdout, _T("Directories are equal\n"));
	}
	else {
		_ftprintf(stdout, _T("Directories are different\n"));
	}
	for (DWORD i = 0; i < nbThreads; i++) {
		CloseHandle(handles[i]);
	}
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

DWORD WINAPI readingThread(LPVOID arg) {
	ARGS* Arg;
	Arg = (ARGS*)arg;
	exploreDirectory(Arg->dirName, GetCurrentThreadId(), Arg->threadNb);
	entries[Arg->threadNb] = _T("");
	EnterCriticalSection(&criticalSection);
	counter++;
	if (counter == nbThreads) {
		PulseEvent(eventComparator);
	}
	LeaveCriticalSection(&criticalSection);
	WaitForSingleObject(eventReaders, INFINITE);
	return 0;
}

VOID exploreDirectory(LPTSTR dirName, DWORD threadId, DWORD threadNb) {
	HANDLE dir;
	LPTSTR pattern, newDirName;
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
		entries[threadNb] = fileInfo.cFileName;
		EnterCriticalSection(&criticalSection);
		counter++;
		if (counter == nbThreads) {
			PulseEvent(eventComparator);
		}
		LeaveCriticalSection(&criticalSection);
		WaitForSingleObject(eventReaders, INFINITE);

		if (FileType(&fileInfo) == TYPE_DIR) {
			newDirName = (LPTSTR)malloc(_tcslen(dirName) + sizeof(TCHAR) + _tcslen(fileInfo.cFileName));
			_tcscpy(newDirName, dirName);
			_tcscat(newDirName, fileInfo.cFileName);
			_tcscat(newDirName, _T("/"));
			exploreDirectory(newDirName, threadId, threadNb);
		}
	} while (FindNextFile(dir, &fileInfo));
	ret = FindClose(dir);
	if (ret == FALSE) {
		_ftprintf(stderr, _T("Error when closing directory %s\n"), dirName);
	}
}

DWORD WINAPI compare(LPVOID arg) {
	while (1) {
		WaitForSingleObject(eventComparator, INFINITE);
		for (DWORD i = 0; i < nbThreads - 1; i++) {
			if (_tcscmp(entries[i], entries[i + 1])) {
				ExitThread(1);
			}
		}
		DWORD i = 0;
		while (i < nbThreads && !_tcscmp(entries[i], _T(""))) {
			i++;
		}
		if (i == nbThreads) {
			ExitThread(0);
		}
		EnterCriticalSection(&criticalSection);
		counter = 0;
		LeaveCriticalSection(&criticalSection);
		PulseEvent(eventReaders);
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