#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

VOID exploreDirectory(LPTSTR dirName, DWORD threadId, DWORD threadNb);
static DWORD FileType(LPWIN32_FIND_DATA fileInfo);

INT _tmain(INT argc, LPTSTR argv[]) {
	if (argc != 3) {
		_ftprintf(stderr, _T("Wrong number of arguments\nUsage : %s name1 name2\n"), argv[0]);
		return 1;
	}
	exploreDirectory(argv[1], 0, 0);

	//Wait some input to allow the user to have time to read the console
	TCHAR a;
	_ftscanf(stdin, _T("%c"), &a);
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
		if (FileType(&fileInfo) == TYPE_DIR) {
			newDirName = (LPTSTR)malloc(_tcslen(dirName) + sizeof(TCHAR) + _tcslen(fileInfo.cFileName));
			_tcscpy(newDirName, dirName);
			_tcscat(newDirName, fileInfo.cFileName);
			_tcscat(newDirName, _T("/"));
			exploreDirectory(newDirName, threadId, threadNb);
			free(newDirName);
		}
	} while (FindNextFile(dir, &fileInfo));
	ret = FindClose(dir);
	if (ret == FALSE) {
		_ftprintf(stderr, _T("Error when closing directory %s\n"), dirName);
	}
	free(pattern);
}


static DWORD FileType(LPWIN32_FIND_DATA fileInfo) {
	BOOL isDir = ((fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
	DWORD fileType = TYPE_FILE;
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
