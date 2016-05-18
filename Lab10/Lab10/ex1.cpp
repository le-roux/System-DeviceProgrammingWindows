#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

VOID exploreDirectory(LPTSTR name);
static DWORD FileType(LPWIN32_FIND_DATA fileInfo);

INT _tmain(INT argc, LPTSTR argv[]) {
	if (argc != 3) {
		_ftprintf(stderr, _T("Wrong number of arguments\nUsage : %s name1 name2\n"), argv[0]);
		return 1;
	}
	exploreDirectory(argv[1]);
	TCHAR a;
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

VOID exploreDirectory(LPTSTR name) {
	LPTSTR pattern = (LPTSTR)malloc(_tcslen(name + sizeof(TCHAR)));
	_tcscpy(pattern, name);
	_tcscat(pattern, _T("*"));
	LPWIN32_FIND_DATA fileInfo = (LPWIN32_FIND_DATA)malloc(sizeof(WIN32_FIND_DATA));
	HANDLE dir = FindFirstFile(pattern, fileInfo);
	if (dir == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when reading directory %s\nError : %i"), name, GetLastError());
		return;
	}
	do {
		if (FileType(fileInfo) == TYPE_DIR) {
			LPTSTR newName = (LPTSTR)malloc(_tcslen(name) + _tcslen(fileInfo->cFileName) + sizeof(TCHAR));
			_tcscpy(newName, name);
			_tcscat(newName, fileInfo->cFileName);
			_tcscat(newName, _T("/"));
			exploreDirectory(newName);
		}
		else if (FileType(fileInfo) == TYPE_FILE) {
			_ftprintf(stdout, _T("file %s\n"), fileInfo->cFileName);
		}
	}while (FindNextFile(dir, fileInfo));
}

static DWORD FileType(LPWIN32_FIND_DATA fileInfo) {
	BOOL isDir = ((fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
	DWORD fileType = TYPE_FILE;
	if (isDir) {
		if (_tcscmp(fileInfo->cFileName, _T(".")) == 0
			|| _tcscmp(fileInfo->cFileName, _T("..")) == 0) {
			fileType = TYPE_DOT;
		} else
			fileType = TYPE_DIR;
	}
	return fileType;
}
