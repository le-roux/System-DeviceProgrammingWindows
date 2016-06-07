/**
 * This program is done to practice the Structured Error Handling mechanism.
 */

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define THREADS_NB 2

HANDLE threadsHandles[THREADS_NB];
DWORD threadsIds[THREADS_NB];

// Used to signal the other thread that an error occured and that it has to stop
volatile BOOLEAN stop = FALSE;

DWORD WINAPI threadFunc1(LPVOID arg);
DWORD WINAPI threadFunc2(LPVOID arg);

INT _tmain(INT argc, LPTSTR argv[]) {
	TCHAR a;
	INT returnCode = 0;

	if (argc != 2) {
		_ftprintf(stdout, _T("Wrong number of arguments\nUsage : %s inputFileName\n"), argv[0]);
		_ftscanf(stdin, _T("%c"), &a);
		return 1;
	}
	__try {
		threadsHandles[0] = CreateThread(NULL, 0, threadFunc1, argv[1], 0, &threadsIds[0]);
		if (threadsHandles[0] == NULL) {
			_ftprintf(stderr, _T("Error %i when creating thread 1\n"), GetLastError());
			returnCode = 2;
			__leave;
		}

		threadsHandles[1] = CreateThread(NULL, 0, threadFunc2, argv[1], 0, &threadsIds[1]);
		if (threadsHandles[1] == NULL) {
			_ftprintf(stderr, _T("Error %i when creating thread 2\n"), GetLastError());
			returnCode = 3;
			__leave;
		}

		WaitForMultipleObjects(THREADS_NB, threadsHandles, TRUE, INFINITE);
	}
	__finally {
		CloseHandle(threadsHandles[0]);
		CloseHandle(threadsHandles[1]);
		_ftscanf(stdin, _T("%c"), &a);
	}
	return returnCode;
}

/* This function reads the file 2 INT at a time, compute their ratio and 
   display the 3 values */
DWORD WINAPI threadFunc1(LPVOID arg) {
	LPTSTR fileName = NULL;
	HANDLE inputFile = NULL;
	DWORD n[2], nRead;
	fileName = (LPTSTR)arg;
	__try {
		__try {
			inputFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (inputFile == INVALID_HANDLE_VALUE)
				RaiseException(0xE0000001, 0, 0, NULL);

			while (!stop && ReadFile(inputFile, n, 2 * sizeof(DWORD), &nRead, NULL) && nRead > 0) {
				if (nRead != 2 * sizeof(DWORD))
					RaiseException(0xE0000002, 0, 0, NULL);
				if (stop)
					__leave;
				_ftprintf(stdout, _T("n1 = %i, n2 = %i, n1/n2 = %i\n"), n[0], n[1], n[0] / n[1]);
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			stop = TRUE;
			_ftprintf(stderr, _T("Error in thread 1\n"));
			__leave;
		}
	}
	__finally {
		CloseHandle(inputFile);
	}
	ExitThread(0);
}

/* This function allocates an array of size the first value stored in the file,
   writes the other values in it, sorts it and finally displays it*/
DWORD WINAPI threadFunc2(LPVOID arg) {
	LPTSTR fileName;
	HANDLE inputFile = NULL, currentHeap = NULL;
	DWORD size, value, nRead, index = 0;
	LPDWORD dynamicArray = NULL;

	fileName = (LPTSTR)arg;
	__try {
		__try {
			inputFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (inputFile == INVALID_HANDLE_VALUE)
				RaiseException(0xE0000003, 0, 0, NULL);

			/* Dynamically create an array of length the first value stored in
			   the input file */
			ReadFile(inputFile, &size, sizeof(DWORD), &nRead, NULL);
			currentHeap = GetProcessHeap();
			if (currentHeap == NULL) {
				_ftprintf(stderr, _T("Error %i when getting current heap\n"), GetLastError());
				__leave;
			}
			dynamicArray = (LPDWORD)HeapAlloc(currentHeap, HEAP_GENERATE_EXCEPTIONS, size * sizeof(DWORD));
			
			// Read the file and store the value in the array
			while (ReadFile(inputFile, &value, sizeof(DWORD), &nRead, NULL) && nRead > 0) {
				if (stop)
					__leave;
				if (index >= size) // Check buffer overflow
					RaiseException(0xE0000004, 0, 0, NULL);
				dynamicArray[index] = value;
				index++;
			}

			// Order the array (bubble sort)
			for (DWORD i = 0; i < size; i++) {
				DWORD tmp = i;
				while (tmp > 0 && dynamicArray[tmp - 1] > dynamicArray[tmp]) {
					if (stop)
						__leave;
					DWORD swap;
					swap = dynamicArray[tmp - 1];
					dynamicArray[tmp - 1] = dynamicArray[tmp];
					dynamicArray[tmp] = swap;
					tmp--;
				}
			}

			// Display the ordered array
			for (DWORD i = 0; i < size; i++) {
				if (stop)
					__leave;
				_ftprintf(stdout, _T("%i "), dynamicArray[i]);
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			stop = TRUE;
			_ftprintf(stderr, _T("Error in thread 2\n"));
			__leave; // All the cleaning work is done in the __finally block
		}
	}
	__finally {
		CloseHandle(inputFile);
		HeapFree(currentHeap, 0, dynamicArray);
		CloseHandle(currentHeap);
	}
	ExitThread(0);
}