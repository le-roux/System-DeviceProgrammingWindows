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

	//Create the sorting threads
	for (DWORD i = 0; i < fileNb; i++) {
		args[i].fileName = argv[i + 1];
		threadsHandles[i] = CreateThread(NULL, 0, sort, &args[i], 0, &threadsIds[i]);
		if (threadsHandles[i] == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, _T("Error when opening %s\n"), argv[i + 1]);
			return 1;
		}
	}
	WaitForMultipleObjects(fileNb, threadsHandles, TRUE, INFINITE);

	//Merge the sorted arrays provided by the sorting threads.
	DWORD size = 0;
	for (DWORD i = 1; i < fileNb; i++) {
		size = args[0].recordNumber + args[i].recordNumber;
		INT* out = (INT*)malloc(size * sizeof(INT));
		merge(args[0].recordNumber, args[0].listPointer, args[i].recordNumber, args[i].listPointer, out);
		args[0].recordNumber = size;
		free(args[0].listPointer);
		args[0].listPointer = out;
	}

	//Close the handles associated to the sorting threads.
	for (DWORD i = 0; i < fileNb; i++) {
		BOOL ret = CloseHandle(threadsHandles[i]);
		if (ret == FALSE) {
			_ftprintf(stderr, _T("Error %i when closing the handle\n"), GetLastError());
		}
	}

	//Write the result (the final sorted array) in the output file
	HANDLE hOut = CreateFile(argv[argc - 1], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when opening %s"), argv[argc - 1]);
		return 1;
	}
	DWORD nOut;
	//Write the number of elements at the beginning.
	WriteFile(hOut, &args[0].recordNumber, sizeof(INT), &nOut, NULL);
	if (nOut != sizeof(INT)) {
		_ftprintf(stderr, _T("Error when writing"));
		BOOL ret = CloseHandle(hOut);
		if (ret == false)
			_ftprintf(stderr, _T("Error %i when closing the handle"), GetLastError());
		return 1;
	}
	//Write the array.
	WriteFile(hOut, args[0].listPointer, size * sizeof(INT), &nOut, NULL);
	if (nOut != size * sizeof(INT)) {
		_ftprintf(stderr, _T("Error when writing"));
		BOOL ret = CloseHandle(hOut);
		if (ret == FALSE)
			_ftprintf(stderr, _T("Error %i when closing the handle"), GetLastError());
		return 1;
	}
	BOOL ret = CloseHandle(hOut);
	if (ret == FALSE)
		_ftprintf(stderr, _T("Error %i when closing the handle"), GetLastError());

	//Display the content of the result file.
	ret = ControlRead(argv[argc - 1]);
	if (ret == FALSE)
		_ftprintf(stderr, _T("Error when displaying the result"));
	
	return 0;
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