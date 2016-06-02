/**
 * This program reads records from a file as soon as they are available
 * and prints them on the standard output.
 *
 * It's done to work with ex3_1.cpp, as the second process.
 */

#include "ex3.h"

TCHAR a;

INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE inputFile = NULL;
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	size.QuadPart = MAX_LEN;

	if (argc != 2) {
		_ftprintf(stderr, _T("Wrong number of arguments\nUsage : %s fileName\n"), argv[0]);
		_ftscanf(stdin, _T("%c"), &a);
	}

	inputFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (inputFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening input file\n"), GetLastError());
		_ftscanf(stdin, _T("%c"), &a);
	}

	while (LockFileEx(inputFile, LOCKFILE_EXCLUSIVE_LOCK, 0, size.LowPart, size.HighPart, &ov)) {
		TCHAR buffer[MAX_LEN];
		DWORD nRead;
		ReadFile(inputFile, buffer, MAX_LEN, &nRead, &ov);
		_ftprintf(stdout, _T("%s\n"), buffer);
		//Unlock the current record
		UnlockFileEx(inputFile, 0, size.LowPart, size.HighPart, &ov);
		ov.Offset += size.LowPart;
		
		if (_tcscmp(buffer, END) == 0)
			break;
	}

	_ftprintf(stdout, _T("THE END\n"));
	CloseHandle(inputFile);
	return 0;
}