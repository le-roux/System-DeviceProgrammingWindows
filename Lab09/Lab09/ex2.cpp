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
	/* Used like a map : index = (true) number of the thread
						 value = value returned by WaitForMultipleObjects(...)
		This array, in combination with WaitForMultipleObjects(...),
		allows to know which thread has terminated.					 
	*/
	INT* translation = (INT*)malloc(fileNb * sizeof(BOOL));
	LPDWORD threadsIds = (LPDWORD)malloc(fileNb * sizeof(DWORD));
	args = (ARGS*)malloc(fileNb * sizeof(ARGS));

	//Create the sorting threads.
	for (DWORD i = 0; i < fileNb; i++) {
		args[i].fileName = argv[i + 1];
		threadsHandles[i] = CreateThread(NULL, 0, sort, &args[i], 0, &threadsIds[i]);
		translation[i] = i;
	}
	DWORD nbThreadsTerminated = 0;
	DWORD firstThreadTerminated;
	DWORD size = 0, res;
	while (nbThreadsTerminated != fileNb) {
		res = WaitForMultipleObjects(fileNb - nbThreadsTerminated, threadsHandles, FALSE, INFINITE);
		if (res == WAIT_FAILED) {
			_ftprintf(stderr, _T("Error when waiting : %i"), GetLastError());
			return 1;
		}
		if (res >= WAIT_OBJECT_0 && res <= (WAIT_OBJECT_0 + fileNb)) {
			//A thread has terminated
			nbThreadsTerminated++;
			DWORD index = 0;
			while (translation[index] != res - WAIT_OBJECT_0) {
				index++;
			}
			DWORD threadJustTerminated = index;
			BOOL ret = CloseHandle(threadsHandles[res - WAIT_OBJECT_0]);
			if (ret == FALSE)
				_ftprintf(stderr, _T("Error %i when closing the file\n"), GetLastError());

			//Update the position of the threads in order to keep data meaningful.
			for (DWORD i = res - WAIT_OBJECT_0; i < fileNb - nbThreadsTerminated; i++) {
					threadsHandles[i] = threadsHandles[i + 1];
			}

			for (DWORD i = threadJustTerminated; i < fileNb; i++) {
				translation[i]--;
			}

			translation[threadJustTerminated] = -1;

			if (nbThreadsTerminated >= 2) {
				//Merge
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
		BOOL ret = CloseHandle(hOut);
		if (ret == FALSE)
			_ftprintf(stderr, _T("Error %i when closing the file\n"), GetLastError());
		return 1;
	}
	WriteFile(hOut, args[firstThreadTerminated].listPointer, size * sizeof(INT), &nOut, NULL);
	if (nOut != size * sizeof(INT)) {
		_ftprintf(stderr, _T("Error when writing"));
		BOOL ret = CloseHandle(hOut);
		if (ret == FALSE)
			_ftprintf(stderr, _T("Error %i when closing the file\n"), GetLastError());
		return 1;
	}
	BOOL ret = CloseHandle(hOut);
	if (ret == FALSE)
		_ftprintf(stderr, _T("Error %i when closing the file\n"), GetLastError());

	//Display the content of the result file.
	ret = ControlRead(argv[argc - 1]);
	if (ret == FALSE)
		_ftprintf(stderr, _T("Error when displaying the result"));

	return 0;
}

DWORD WINAPI sort(LPVOID args) {
	ARGS* arg = (ARGS*)args;
	HANDLE hIn;
	hIn = CreateFile(arg->fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when opening %s"), arg->fileName);
		ExitThread(1);
	}
	DWORD recordNb, nOut;
	ReadFile(hIn, &recordNb, sizeof(DWORD), &nOut, NULL);
	if (nOut != sizeof(DWORD)) {
		_ftprintf(stderr, _T("Error when reading the file\n"));
		ExitThread(1);
	}

	arg->recordNumber = recordNb;
	arg->listPointer = (INT*)malloc(recordNb * sizeof(INT));
	ReadFile(hIn, arg->listPointer, recordNb * sizeof(INT), &nOut, NULL);
	if (nOut != recordNb * sizeof(INT)) {
		_ftprintf(stderr, _T("Error when reading file"));
		ExitThread(1);
	}

	BOOL ret = CloseHandle(hIn);
	if (ret == FALSE)
		_ftprintf(stderr, _T("Error %i when closing the file"), GetLastError());

	for (DWORD i = 0; i < recordNb; i++) {
		DWORD index = i;
		DWORD tmp;
		//Bubble sort
		while (index > 0 && arg->listPointer[index - 1] > arg->listPointer[index]) {
			tmp = arg->listPointer[index - 1];
			arg->listPointer[index - 1] = arg->listPointer[index];
			arg->listPointer[index] = tmp;
			index--;
		}
	}
	ExitThread(0);
}

//Merge list1 and list2 in a single array
VOID merge(DWORD size1, INT* list1, DWORD size2, INT* list2, INT* out) {
	DWORD i = 0, j = 0, index = 0;
	while (i < size1 && j < size2) {
		if (list1[i] < list2[j]) {
			out[index] = list1[i];
			i++;
		}
		else {
			out[index] = list2[j];
			j++;
		}
		index++;
	}
	while (i < size1) {
		out[index] = list1[i];
		i++;
		index++;
	}
	while (j < size2) {
		out[index] = list2[j];
		j++;
		index++;
	}
}

BOOL ControlRead(LPTSTR lpfileName) {
	HANDLE hOut = CreateFile(lpfileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when reopening the output file"));
		return FALSE;
	}

	DWORD nIn;
	HANDLE console;

	console = GetStdHandle(STD_OUTPUT_HANDLE);
	if (console == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when closing the handle"), GetLastError());
		return FALSE;
	}

	INT read;
	//Read the binary file and print it to the console
	while (ReadFile(hOut, &read, sizeof(INT), &nIn, NULL) && nIn > 0) {
		_ftprintf(stdout, _T("%i "), read);
		if (nIn != sizeof(INT)) {
			_ftprintf(stderr, _T("Error writing to console"));
			BOOL ret = CloseHandle(hOut);
			if (ret == FALSE)
				_ftprintf(stderr, _T("Error %i when closing the handle"), GetLastError());
			return FALSE;
		}
	}

	//Wait some input to allow the user to have time to read the console
	TCHAR a;
	_ftscanf(stdin, _T("%c"), &a);

	BOOL ret = CloseHandle(hOut);
	if (ret == FALSE)
		_ftprintf(stderr, _T("Error %i when closing the handle"), GetLastError());

	return TRUE;
}