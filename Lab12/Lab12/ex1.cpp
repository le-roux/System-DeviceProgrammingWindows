#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define THREADS_NB 4

HANDLE inputSem[THREADS_NB], outputSem;
DWORD inputValue;
TCHAR a;

DWORD WINAPI threadFunc(LPVOID arg);

INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE threadsHandles[THREADS_NB], inputFile;
	DWORD threadsIds[THREADS_NB], nRead;

	if (argc != 2) {
		_ftprintf(stdout, _T("Wrong number of arguments\nUsage : %s inputFileName\n"), argv[0]);
		_ftscanf(stdin, _T("%c"), &a);
		return 1;
	}

	for (INT i = 0; i < THREADS_NB; i++)
		inputSem[i] = CreateSemaphore(NULL, 0, 1, NULL);
	outputSem = CreateSemaphore(NULL, 0, THREADS_NB, NULL);

	LPDWORD threadPos;
	for (INT i = 0; i < THREADS_NB; i++) {
		threadPos = (LPDWORD)malloc(sizeof(DWORD));
		*threadPos = i;
		threadsHandles[i] = CreateThread(NULL, 0, threadFunc, threadPos, 0, &threadsIds[i]);
		if (threadsHandles[i] == NULL) {
			_ftprintf(stdout, _T("Error %i when creating thread number %i\n"), GetLastError(), i);
			_ftscanf(stdin, _T("%c"), &a);
			return 1;
		}
	}

	inputFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (inputFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stdout, _T("Error %i when opening input file\n"), GetLastError());
		_ftscanf(stdin, _T("%c"), &a);
		return 1;
	}

	while (ReadFile(inputFile, &inputValue, sizeof(DWORD), &nRead, NULL) && nRead > 0) {
		for (INT i = 0; i < THREADS_NB; i++)
			ReleaseSemaphore(inputSem[i], 1, NULL);
		for (INT i = 0; i < THREADS_NB; i++) {
			WaitForSingleObject(outputSem, INFINITE);
		}
	}

	for (INT i = 0; i < THREADS_NB; i++) {
		TerminateThread(threadsHandles[i], 0);
	}
	CloseHandle(inputSem);
	CloseHandle(outputSem);
	CloseHandle(inputFile);
	_ftprintf(stdout, _T("================================================\n"));
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

DWORD WINAPI threadFunc(LPVOID arg) {
	DWORD threadPos;
	DWORDLONG sum = 0, product = 1, factorial = 1;
	threadPos = *(LPDWORD)arg;
	while (1) {
		WaitForSingleObject(inputSem[threadPos], INFINITE);
		switch (threadPos) {
			case 0: {
				sum += inputValue;
				_ftprintf(stdout, _T("Thread 1, sum = %lld\n"), sum);
				break;
			}
			case 1: {
				product *= inputValue;
				_ftprintf(stdout, _T("Thread 2, product = %lld\n"), product);
				break;
			}
			case 2: {
				factorial = 1;
				for (DWORD i = 1; i <= inputValue; i++)
					factorial *= i;
				_ftprintf(stdout, _T("Thread 3, factorial = %lld\n"), factorial);
				break;
			}
			default: {
				for (INT i = 0; i < inputValue; i++)
					_ftprintf(stdout, _T("#"));
			}
		}
		ReleaseSemaphore(outputSem, 1, NULL);
	}
	ExitThread(0);
}