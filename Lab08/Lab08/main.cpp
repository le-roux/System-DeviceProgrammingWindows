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
	TCHAR name[30];
	TCHAR surname[30];
	int mark;
} Record;

void ReadRecord(HANDLE hIn, int lineNumber) {
	Record record;
	DWORD nIn;
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	LARGE_INTEGER filePos;
	filePos.QuadPart = (lineNumber - 1) * sizeof(Record);
	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;
	ReadFile(hIn, &record, sizeof(Record), &nIn, &ov);
	fprintf(stdout, "%i %i %s %s %i\n", record.lineNumber, record.registerNumber, record.name, record.surname, record.mark);
}

void WriteRecord(HANDLE hIn, int lineNumber, Record record) {
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	LARGE_INTEGER filePos;
	filePos.QuadPart = (lineNumber - 1) * sizeof(Record);
	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;
	DWORD nOut;
	WriteFile(hIn, &record, sizeof(Record), &nOut, &ov);
}

int _tmain(int argc, LPTSTR argv[]) {
	
	HANDLE hIn;

	hIn = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error when opening file"));
		return 1;
	}

	BOOL cont = true;

	TCHAR command = 0;
	while(cont) {
		if(command != '\n')
			_ftprintf(stdout, _T("User choice : "));
		_ftscanf(stdin, _T("%c"), &command);
		if (command == 'E') {
			cont = false;
		} else if (command == 'R') {
			int lineNumber;
			_ftscanf(stdin, _T("%i"), &lineNumber);
			if (lineNumber > 1)
				ReadRecord(hIn, lineNumber);
			else
				_ftprintf(stdin, _T("Wrong record number\n"));
		} else if (command == 'W') {
			Record newRecord;
			_ftscanf(stdin, _T("%i %i %s %s %i"), &newRecord.lineNumber, &newRecord.registerNumber, &newRecord.name, &newRecord.surname, &newRecord.mark);
			WriteRecord(hIn, newRecord.lineNumber, newRecord);
		}
	}

	return 0;
}