#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

#define VERSION_B

typedef struct ARGS {
	LPTSTR dirName;
	DWORD threadNb;
} ARGS;

static DWORD FileType(LPWIN32_FIND_DATA fileInfo);
DWORD WINAPI readingThread(LPVOID arg);
VOID exploreDirectory(LPTSTR dirName, DWORD threadId);
DWORD WINAPI readingThreadB(LPVOID arg);
VOID exploreDirectoryB(LPTSTR dirName, DWORD threadId, HANDLE outputFile);

HANDLE* outputFiles;
ARGS* arguments;

INT _tmain(INT argc, LPTSTR argv[]) {
	if (argc == 1) {
		_ftprintf(stderr, _T("Wrong number of arguments\nUsage : %s + N directories"), argv[0]);
		return 1;
	}

	DWORD nbThreads, nOut, ret;
	HANDLE* handles;
	TCHAR outputName[100], output[100];
	DWORD* threadsId;
	nbThreads = argc - 1;
	handles = (HANDLE*)malloc(nbThreads * sizeof(HANDLE));
	threadsId = (DWORD*)malloc(nbThreads * sizeof(DWORD));
#ifdef VERSION_B
	outputFiles = (HANDLE*)malloc(nbThreads * sizeof(HANDLE));
	arguments = (ARGS*)malloc(nbThreads * sizeof(ARGS));
#endif
	for (DWORD i = 0; i < nbThreads; i++) {
		#ifdef VERSION_B
		arguments[i].dirName = argv[i + 1];
		arguments[i].threadNb = i;
		handles[i] = CreateThread(NULL, 0, readingThreadB, &arguments[i], CREATE_SUSPENDED, &threadsId[i]);
		_stprintf(outputName, _T("%d"), threadsId[i]);
		outputFiles[i] = CreateFile(outputName, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (outputFiles[i] == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, _T("Error %i when creating file %s"), GetLastError(), outputName);
			return 1;
		}
		ResumeThread(handles[i]);
		#else
		handles[i] = CreateThread(NULL, 0, readingThread, argv[i + 1], 0, &threadsId[i]);
		#endif
	}
	WaitForMultipleObjects(nbThreads, handles, TRUE, INFINITE);

	#ifdef VERSION_B
	HANDLE console;
	DWORD size;
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	if (console == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening the console\n"), GetLastError());
		return 1;
	}
	for (DWORD i = 0; i < nbThreads; i++) {
		_stprintf(outputName, _T("%d"), threadsId[i]);
		ret = SetFilePointer(outputFiles[i], 0, NULL, FILE_BEGIN);
		if (ret != 0) {
			_ftprintf(stderr, _T("Error %i when moving file pointer\n"), GetLastError());
			break;
		}
		while (1) {
			if (ReadFile(outputFiles[i], output, 100, &nOut, NULL)) {
				if (nOut > 0)
					WriteConsole(console, output, nOut, &out, NULL);
				else
					break;
			}
			else {
				_ftprintf(stderr, _T("Error %i when reading file\n"), GetLastError());
				break;
			}
		}
		CloseHandle(outputFiles[i]);
	}
	CloseHandle(console);
	#endif
	TCHAR a;
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

//Version A
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

//Version B
DWORD WINAPI readingThreadB(LPVOID arg) {
	ARGS* Arg;
	Arg = (ARGS*)arg;
	exploreDirectoryB(Arg->dirName, GetCurrentThreadId(), outputFiles[Arg->threadNb]);
	return 0;
}

VOID exploreDirectoryB(LPTSTR dirName, DWORD threadId, HANDLE outputFile) {
	HANDLE dir;
	LPTSTR pattern;
	TCHAR newDirName[500], output[100];
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
			_stprintf(output, _T("Thread %i : file %s\n"), threadId, fileInfo.cFileName);
			WriteFile(outputFile, output, 2 * _tcslen(output), &ret, NULL);
			if (ret != 2 * _tcslen(output)) {
				_ftprintf(stderr, _T("Error when writing the output\n"));
				FindClose(dir);
				return;
			}
		}
		else if (FileType(&fileInfo) == TYPE_DIR) {
			_tcscpy(newDirName, dirName);
			_tcscat(newDirName, fileInfo.cFileName);
			_tcscat(newDirName, _T("/"));
			exploreDirectoryB(newDirName, threadId, outputFile);
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