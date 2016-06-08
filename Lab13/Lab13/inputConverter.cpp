#include "lab13.h"
#include <stdio.h>

INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE outputFile;
	FILE* inputFile;
	Record record;
	DWORD nOut;
	if (argc != 3) {
		return 1;
	}

	inputFile = _tfopen(argv[1], _T("r"));
	if (inputFile == INVALID_HANDLE_VALUE) {
		return 1;
	}

	outputFile = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (outputFile == INVALID_HANDLE_VALUE) {
		fclose(inputFile);
		return 1;
	}

	while (_ftscanf(inputFile, _T("%s %s %s"), record.directoryName, record.inputFileName, record.outputName) > 0) {
		WriteFile(outputFile, &record, sizeof(Record), &nOut, NULL);
		if (sizeof(Record) != nOut) {
			fclose(inputFile);
			CloseHandle(outputFile);
			return 1;
		}
	}

	fclose(inputFile);
	CloseHandle(outputFile);

	outputFile = CreateFile(argv[2], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	while (ReadFile(outputFile, &record, sizeof(Record), &nOut, NULL) && nOut > 0) {
		_ftprintf(stdout, _T("%s %s %s\n"), record.directoryName, record.inputFileName, record.outputName);
	}
	CloseHandle(outputFile);
}