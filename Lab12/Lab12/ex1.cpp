/**
 * Implements a simple producer-consumer scheme with the following characteristics :
 *		- single producer : the main thread
 *		- multiple consumers : the secondary threads
 *		- synchronization performed using either semaphores or events
 */

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

//Number of threads to run
#define THREADS_NB 4

/* Define either VERSION_A or VERSION_B according to the type
 * of synchronisation object you want to use
 *	VERSION_A : semaphores
 *	VERSION_B : events
 */
#define VERSION_B

#ifdef VERSION_A
	HANDLE inputSem[THREADS_NB], outputSem;
#endif

#ifdef VERSION_B
	HANDLE inputEvent, outputEvent[THREADS_NB];
#endif

//Value read from the file
volatile DWORD inputValue;

/*Trash value used for leaving the console opened at
  the end*/
TCHAR a;

DWORD WINAPI threadFunc(LPVOID arg);

INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE threadsHandles[THREADS_NB], inputFile = NULL;
	DWORD threadsIds[THREADS_NB], nRead;

	if (argc != 2) {
		_ftprintf(stdout, _T("Wrong number of arguments\nUsage : %s inputFileName\n"), argv[0]);
		_ftscanf(stdin, _T("%c"), &a);
		return 1;
	}

	//Initialize synchronization objects
	#ifdef VERSION_A
		for (INT i = 0; i < THREADS_NB; i++)
			inputSem[i] = CreateSemaphore(NULL, 0, 1, NULL);
		outputSem = CreateSemaphore(NULL, 0, THREADS_NB, NULL);
	#endif
	#ifdef VERSION_B
		inputEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		for (INT i = 0; i < THREADS_NB; i++)
			outputEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	#endif

	//Create the threads
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

	/* Used to be sure that all the secondary threads have been activated and wait
	   on the event before the main thread triggers it */
	#ifdef VERSION_B
		Sleep(100);
	#endif

	//Read the file
	while (ReadFile(inputFile, (LPVOID)&inputValue, sizeof(DWORD), &nRead, NULL) && nRead > 0) {
		#ifdef VERSION_A
			for (INT i = 0; i < THREADS_NB; i++)
				ReleaseSemaphore(inputSem[i], 1, NULL);
			for (INT i = 0; i < THREADS_NB; i++) {
				WaitForSingleObject(outputSem, INFINITE);
			}
		#endif
		#ifdef VERSION_B
			PulseEvent(inputEvent);
			WaitForMultipleObjects(THREADS_NB, outputEvent, TRUE, INFINITE);
		#endif
	}

	for (INT i = 0; i < THREADS_NB; i++) {
		TerminateThread(threadsHandles[i], 0);
	}

	//Clean the synchronization objects used
	#ifdef VERSION_A
		for (INT i = 0; i < THREADS_NB; i++)
			CloseHandle(inputSem[i]);
		CloseHandle(outputSem);
	#endif
	#ifdef VERSION_B
		CloseHandle(inputEvent);
		CloseHandle(outputEvent);
	#endif

	CloseHandle(inputFile);

	//Display a special line to indicate the end of the program
	_ftprintf(stdout, _T("================================================\n"));
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

DWORD WINAPI threadFunc(LPVOID arg) {
	DWORD threadPos;
	DWORDLONG sum = 0, product = 1, factorial = 1;
	threadPos = *(LPDWORD)arg;
	while (1) {
		#ifdef VERSION_A
			WaitForSingleObject(inputSem[threadPos], INFINITE);
		#endif
	#ifdef VERSION_B
			WaitForSingleObject(inputEvent, INFINITE);
	#endif
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
				_ftprintf(stdout, _T("Thread 4 : "));
				for (INT i = 0; i < inputValue; i++)
					_ftprintf(stdout, _T("#"));
				_ftprintf(stdout, _T("\n"));
			}
		}

		/* Release the main thread (each secondary thread must perform this step
		   in order that the main continues its execution */
		#ifdef VERSION_A
			ReleaseSemaphore(outputSem, 1, NULL);
		#endif
		#ifdef VERSION_B
			SetEvent(outputEvent[threadPos]);
		#endif
	}
	ExitThread(0);
}