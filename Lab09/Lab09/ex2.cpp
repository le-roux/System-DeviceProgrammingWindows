#define UNICODE
#define _UNICODE

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "lab09.h"

INT _tmain(INT argc, LPTSTR argv[]) {
	DWORD fileNb = argc - 2;
	HANDLE* threadsHandles = (HANDLE*)malloc(fileNb * sizeof(HANDLE));
	LPDWORD threadsIds = (LPDWORD)malloc(fileNb * sizeof(DWORD));
	args = (ARGS*)malloc(fileNb * sizeof(ARGS));
	for (DWORD i = 0; i < fileNb; i++) {
		args[i].fileName = argv[i + 1];
		threadsHandles[i] = CreateThread(NULL, 0, sort, &args[i], 0, &threadsIds[i]);
	}
	DWORD nbThreadsTerminated = 0;
	DWORD firstThreadTerminated;
	DWORD size = 0;
	while (nbThreadsTerminated != fileNb) {
		DWORD res = WaitForMultipleObjects(fileNb, threadsHandles, FALSE, INFINITE);
		if (res >= WAIT_OBJECT_0 && res <= (WAIT_OBJECT_0 + fileNb)) {
			//A thread has terminated
			nbThreadsTerminated++;
			if (nbThreadsTerminated >= 2) {
				//Merge
				DWORD threadJustTerminated = res - WAIT_OBJECT_0;
				size = args[firstThreadTerminated].recordNumber + args[threadJustTerminated].recordNumber;
				INT* out = (INT*)malloc(size * sizeof(INT));
				merge(args[firstThreadTerminated].recordNumber, args[firstThreadTerminated].listPointer, args[threadJustTerminated].recordNumber, args[threadJustTerminated].listPointer, out);
				args[firstThreadTerminated].recordNumber = size;
				free(args[firstThreadTerminated].listPointer);
				args[firstThreadTerminated].listPointer = out;
			} else {
				firstThreadTerminated = res - WAIT_OBJECT_0;
			}
		}
	}
	
	//Write it in the output file
	HANDLE hOut = CreateFile(argv[argc - 1], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when opening %s"), argv[argc - 1]);
		return 1;
	}
	DWORD nOut;
	WriteFile(hOut, &args[firstThreadTerminated].recordNumber, sizeof(INT), &nOut, NULL);
	if (nOut != sizeof(INT)) {
		_ftprintf(stderr, _T("Error when writing"));
		CloseHandle(hOut);
		return 1;
	}
	WriteFile(hOut, args[firstThreadTerminated].listPointer, size * sizeof(INT), &nOut, NULL);
	if (nOut != size * sizeof(INT)) {
		_ftprintf(stderr, _T("Error when writing"));
		CloseHandle(hOut);
		return 1;
	}
	CloseHandle(hOut);


	//Read the binary output file to see if everything worked well
	hOut = CreateFile(argv[argc - 1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when reopening the output file"));
		return 1;
	}

	DWORD nIn;
	HANDLE console;

	console = GetStdHandle(STD_OUTPUT_HANDLE);
	INT read;
	//Read the binary file and print it to the console
	while (ReadFile(hOut, &read, sizeof(INT), &nIn, NULL) && nIn > 0) {
		_ftprintf(stdout, _T("%i "), read);
		if (nIn != sizeof(INT)) {
			_ftprintf(stderr, _T("Error writing to console"));
			CloseHandle(hOut);
			return 1;
		}
	}

	//Wait some input to allow the user to have time to read the console
	TCHAR a;
	_ftscanf(stdin, _T("%c"), &a);

	CloseHandle(hOut);
	return 0;
}