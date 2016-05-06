#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

//Simple structure representing the data we have to convert
typedef struct Record {
	int lineNumber;
	int registerNumber;
	char name[30];
	char surname[30];
	int mark;
} Record;

int ex2(int argc, LPTSTR argv[]) {
	FILE* inputFile;
	HANDLE hOut;
	DWORD nOut;
	
	if (argc != 3) {
		_ftprintf(stderr, _T("Wrong number of arguments.\n Usage : %s inputFile outputFile"), argv[0]);
		return 1;
	}
	
	inputFile = fopen(argv[1], "r");
	
	hOut = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error when opening second file");
		fclose(inputFile);
		return 1;
	}
	
	Record  record;
	//Actual conversion froò ASCII to binary format
	while (_ftscanf(inputFile, _T("%i %i %s %s %i"), &record.lineNumber, &record.registerNumber, &record.name, &record.surname, &record.mark) > 0) {
		WriteFile(hOut, &record, sizeof(Record), &nOut, NULL);
	}
	
	//Release the resources
	fclose(inputFile);
	CloseHandle(hOut);
	
	//Check that it has been convert properly

	hOut = CreateFile(argv[2], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when reopening the output file"));
		return 1;
	}
	
	DWORD nIn;
	HANDLE console;
	
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	
	//Read the binary file and print it to the console
	while(ReadFile(hOut, &record, sizeof(Record), &nIn, NULL) && nIn > 0) {
		WriteFile(console, &record, sizeof(Record), &nOut, NULL);
		_ftprintf(stdout, _T("\n%i %i %s %s %i\n"), record.lineNumber, record.registerNumber, record.name, record.surname, record.mark);
		if (nIn != nOut) {
			_ftprintf(stderr, _T("Error writing to console"));
			CloseHandle(hOut);
			return 1;
		}
	}

	//Wait some input to allow the user to have time to read the console
	char a;
	_ftscanf(stdin, _T("%c"), &a);
	
	CloseHandle(hOut);
	return 0;
}