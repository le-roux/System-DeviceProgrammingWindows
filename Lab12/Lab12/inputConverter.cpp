#define UNICODE
#define _UNICODE

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <errno.h>

INT _tmain(INT argc, LPTSTR argv[]) {
	FILE* inputFile;
	HANDLE hOut;
	DWORD nOut;

	if (argc != 3) {
		_ftprintf(stderr, _T("Wrong number of arguments.\n Usage : %s inputFile outputFile"), argv[0]);
		return 1;
	}

	inputFile = _tfopen(argv[1], _T("r"));
	if (inputFile == NULL) {
		_ftprintf(stderr, _T("error when opening input file, errno = %i\n"), errno);
		return 1;
	}

	hOut = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error %i when opening second file", GetLastError());
		fclose(inputFile);
		return 1;
	}

	DWORD value;
	//Actual conversion from ASCII to binary format
	while (_ftscanf(inputFile, _T("%i"), &value) > 0) {
		WriteFile(hOut, &value, sizeof(DWORD), &nOut, NULL);
	}

	//Release the resources
	fclose(inputFile);
	CloseHandle(hOut);

	//Check that it has been convert properly

	hOut = CreateFile(argv[2], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when reopening the output file"));
		return 1;
	}

	DWORD nIn;
	HANDLE console;

	console = GetStdHandle(STD_OUTPUT_HANDLE);

	//Read the binary file and print it to the console
	while (ReadFile(hOut, &value, sizeof(DWORD), &nIn, NULL) && nIn > 0) {
		_ftprintf(stdout, _T("%i\n"), value);
		if (nIn != nOut) {
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