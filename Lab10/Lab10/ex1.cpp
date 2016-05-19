#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

VOID exploreDirectory(LPTSTR name, LPTSTR DestinationDirectory);
static DWORD FileType(LPWIN32_FIND_DATA fileInfo);
VOID addSlash(LPTSTR name);

INT _tmain(INT argc, LPTSTR argv[]) {
	if (argc != 3) {
		_ftprintf(stderr, _T("Wrong number of arguments\nUsage : %s name1 name2\n"), argv[0]);
		return 1;
	}
	exploreDirectory(argv[1], argv[2]);

	//Wait some input to allow the user to have time to read the console
	TCHAR a;
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

VOID exploreDirectory(LPTSTR name, LPTSTR DestinationDirectory) {
	LPTSTR pattern = (LPTSTR)malloc(_tcslen(name + sizeof(TCHAR)));
	TCHAR newSource[500], newDestination[500];
	BOOL ret;
	_tcscpy(pattern, name);
	addSlash(pattern);
	_tcscat(pattern, _T("*"));
	WIN32_FIND_DATA fileInfo;
	HANDLE dir = FindFirstFile(pattern, &fileInfo);
	if (dir == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when reading directory %s\nError : %i"), name, GetLastError());
		return;
	}

	ret = CreateDirectory(DestinationDirectory, NULL);
	if (ret == FALSE) {
		_ftprintf(stderr, _T("Impossible to create directory %s\n"), DestinationDirectory);
		return;
	}

	do {
		if (FileType(&fileInfo) == TYPE_DIR) {
			_tcscpy(newSource, name);
			_tcscat(newSource, fileInfo.cFileName);
			addSlash(newSource);

			_tcscpy(newDestination, DestinationDirectory);
			_tcscat(newDestination, fileInfo.cFileName);
			addSlash(newDestination);

			exploreDirectory(newSource, newDestination);

		} else if (FileType(&fileInfo) == TYPE_FILE) {
			_tcscpy(newSource, name);
			_tcscat(newSource, fileInfo.cFileName);
			
			_tcscpy(newDestination, DestinationDirectory);
			_tcscat(newDestination, fileInfo.cFileName);

			ret = CopyFile(newSource, newDestination, FALSE);
			if (ret == FALSE) {
				_ftprintf(stderr, _T("Error %i when copying file\n"), GetLastError());
				return;
			}
			_ftprintf(stdout, _T("file %s\n"), fileInfo.cFileName);
		}
	} while (FindNextFile(dir, &fileInfo));
	ret = FindClose(dir);
	if (ret == FALSE) {
		_ftprintf(stderr, _T("Error when closing directory %s\n"), name);
	}
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

VOID addSlash(LPTSTR name) {
	DWORD length;
	length = _tcslen(name);
	if (name[length - 1] != _T('/')) {
		_tcscat(name, _T("/"));
	}
}

BOOL CopyAndModifyFile(LPTSTR source, LPTSTR destination) {
	HANDLE srcFile, destFile;
	
	srcFile = CreateFile(source, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (srcFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening file %s\n"), GetLastError(), source);
		return FALSE;
	}

	destFile = CreateFile(destination, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (destFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when creating file %s\n"), GetLastError(), destination);
		CloseHandle(srcFile);
		return FALSE;
	}

	return TRUE;
}