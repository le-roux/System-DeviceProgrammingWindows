#define UNICODE
#define _UNICODE

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

//Version A
void ReadRecordWithFilePointer(HANDLE hIn, int lineNumber) {
	Record record;
	DWORD nIn;
	LONG offset = (lineNumber - 1) * sizeof(Record);
	DWORD move = SetFilePointer(hIn, offset, NULL, FILE_BEGIN);
	if (0xFFFFFFFF == move) {
		_ftprintf(stderr, _T("Error %i\n"), GetLastError());
		return;
	}
	ReadFile(hIn, &record, sizeof(Record), &nIn, NULL);
	_ftprintf(stdout, _T("%i %i %s %s %i\n"), record.lineNumber, record.registerNumber, record.name, record.surname, record.mark);
}

//Version B
void ReadRecordWithOverlapped(HANDLE hIn, int lineNumber) {
	Record record;
	DWORD nIn;
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	LARGE_INTEGER filePos;
	filePos.QuadPart = (lineNumber - 1) * sizeof(Record);
	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;
	ReadFile(hIn, &record, sizeof(Record), &nIn, &ov);
	_ftprintf(stdout, _T("%i %i %s %s %i\n"), record.lineNumber, record.registerNumber, record.name, record.surname, record.mark);
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

//Version C
void ReadRecordWithLocking(HANDLE hIn, int lineNumber) {
	Record record;
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	LARGE_INTEGER offset;
	offset.QuadPart = (lineNumber - 1) * sizeof(Record);
	ov.Offset = offset.LowPart;
	ov.OffsetHigh = offset.HighPart;
	LARGE_INTEGER lockRange;
	lockRange.QuadPart = sizeof(Record);
	BOOLEAN result;
	result = LockFileEx(hIn, 0, 0, lockRange.LowPart, lockRange.HighPart, &ov);
	if (result) {
		DWORD nOut;
		ReadFile(hIn, &record, sizeof(Record), &nOut, &ov);
		_ftprintf(stdout, _T("%i %i %s %s %i\n"), record.lineNumber, record.registerNumber, record.name, record.surname, record.mark);
		result = UnlockFileEx(hIn, 0, lockRange.LowPart, lockRange.HighPart, &ov);
		if (!result) {
			_ftprintf(stderr, _T("Error when releasing the lock\n"));
		}
	} else {
		_ftprintf(stderr, _T("Error when acquiring the lock\n"));
	}
}

void WriteRecordWithLocking(HANDLE hIn, int lineNumber, Record record) {
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	LARGE_INTEGER filePos;
	filePos.QuadPart = (lineNumber - 1) * sizeof(Record);
	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;
	DWORD nOut;
	LARGE_INTEGER lockRange;
	lockRange.QuadPart = sizeof(Record);
	BOOLEAN result;
	result = LockFileEx(hIn, LOCKFILE_EXCLUSIVE_LOCK, 0, lockRange.LowPart, lockRange.HighPart, &ov);
	if (result) {
		WriteFile(hIn, &record, sizeof(Record), &nOut, &ov);
		result = UnlockFileEx(hIn, 0, lockRange.LowPart, lockRange.HighPart, &ov);
		if (!result)
			_ftprintf(stderr, _T("Error when releasing the lock\n"));
	} else {
		_ftprintf(stderr, _T("Error when acquiring the lock\n"));
	}
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
			if (lineNumber >= 1)
				ReadRecordWithLocking(hIn, lineNumber);
			else
				_ftprintf(stdin, _T("Wrong record number\n"));
		} else if (command == 'W') {
			Record newRecord;
			INT lineNumber;
			_ftscanf(stdin, _T("%i"), &lineNumber);
			_ftscanf(stdin, _T("%i %i %s %s %i"), &newRecord.lineNumber, &newRecord.registerNumber, &newRecord.name, &newRecord.surname, &newRecord.mark);
			WriteRecordWithLocking(hIn, lineNumber, newRecord);
		}
	}

	return 0;
}