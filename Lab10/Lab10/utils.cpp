#include "utils.h"

DWORD FileType(LPWIN32_FIND_DATA fileInfo) {
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

LPTSTR addFinalSlash(LPTSTR input) {
	DWORD length;
	LPTSTR output;
	length = _tcslen(input);
	if (input[length - 1] == '/') {
		return input;
	}
	else {
		output = (LPTSTR)malloc(length * sizeof(TCHAR));
		_tcscpy(output, input);
		_tcscat(output, _T("/"));
		return output;
	}
}