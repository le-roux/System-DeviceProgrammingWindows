#include "lab13.h"

CRITICAL_SECTION inputFileCS, outputUpdateCS;
BOOLEAN stop = FALSE;
HANDLE event;
DWORD recordNumber[MAX_LENGTH], outputIndex;
DWORD offset = 0;
INT M;
OUTPUT_FILE outputFiles[MAX_LENGTH];

INT _tmain(INT argc, LPTSTR argv[]) {
	DWORD N;
	LPHANDLE threadsHandles;
	LPDWORD threadsIds;
	TCHAR a;
	
	if (argc != 4) {
		return 1;
	}

	N = _tstoi(argv[2]);
	M = _tstoi(argv[3]);

	

	InitializeCriticalSection(&inputFileCS);
	InitializeCriticalSection(&outputUpdateCS);
	event = CreateEvent(NULL, TRUE, FALSE, NULL);
	for (INT i = 0; i < MAX_LENGTH; i++)
		recordNumber[i] = 0;

	threadsHandles = (LPHANDLE)malloc((N + 1) * sizeof(HANDLE));
	threadsIds = (LPDWORD)malloc((N + 1) * sizeof(DWORD));

	for (DWORD i = 0; i < N; i++) {
		threadsHandles[i] = CreateThread(NULL, 0, threadFunction, argv[1], 0, &threadsIds[i]);
	}
	threadsHandles[N] = CreateThread(NULL, 0, updateFunction, NULL, 0, &threadsIds[N]);
	WaitForMultipleObjects(N + 1, threadsHandles, TRUE, INFINITE);
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

DWORD WINAPI threadFunction(LPVOID arg) {
	HANDLE inputFile, directory, file;
	Record record;
	DWORD nIn, index = 0;
	WIN32_FIND_DATA fileInfo;
	TCHAR pattern[2 * MAX_LENGTH];
	LPTSTR inputFileName;
	inputFileName = (LPTSTR)arg;
	while (!stop) {
		BOOLEAN ret;
		EnterCriticalSection(&inputFileCS);
		inputFile = CreateFile(inputFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (inputFile == INVALID_HANDLE_VALUE) {
			return 1;
		}
		SetFilePointer(inputFile, offset * sizeof(Record), NULL, FILE_BEGIN);
		ret = ReadFile(inputFile, &record, sizeof(Record), &nIn, NULL);
		while (outputFiles[index].fileName != NULL) {
			if (_tcscmp(outputFiles[index].fileName, record.outputName) == 0)
				break;
			index++;
		}
		if (outputFiles[index].fileName == NULL) {
			outputFiles[index].fileName = record.outputName;
			InitializeCriticalSection(&outputFiles[index].cs);
		}
		offset++;
		CloseHandle(inputFile);
		if (sizeof(Record) != nIn) {
			LeaveCriticalSection(&inputFileCS);
			stop = TRUE;
			ExitThread(1);
		}
		LeaveCriticalSection(&inputFileCS);

		_tcscpy(pattern, record.directoryName);
		_tcscat(pattern, record.inputFileName);
		directory = FindFirstFile(pattern, &fileInfo);
		do {
			if (GetFileType(&fileInfo) == TYPE_FILE) {
				DWORD nRead, nWrite;
				OutputRecord rec;
				TCHAR read, filePath[2 * MAX_LENGTH];

				rec.charCount = 0;
				rec.lineCount = 0;
				_tcscpy(rec.fileName, fileInfo.cFileName);
				_tcscpy(filePath, record.directoryName);
				_tcscat(filePath, fileInfo.cFileName);
				file = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file == INVALID_HANDLE_VALUE) {
					_ftprintf(stderr, _T("Error %i when opening input file %s\n"), GetLastError(), filePath);
					ExitThread(1);
				}
				while (ReadFile(file, &read, sizeof(TCHAR), &nRead, NULL) && nRead > 0) {
					rec.charCount++;
					if (read == '\n')
						rec.lineCount++;
				}
				CloseHandle(file);
				EnterCriticalSection(&outputFiles[index].cs);
				file = CreateFile(record.outputName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file == INVALID_HANDLE_VALUE) {
					_ftprintf(stderr, _T("Error %i when opening output file %s (index = %i)\n"), GetLastError(), record.outputName, index);
					ExitThread(1);
				}
				SetFilePointer(file, 0, NULL, FILE_END);
				WriteFile(file, &rec, sizeof(OutputRecord), &nWrite, NULL);
				recordNumber[index]++;
				CloseHandle(file);
				LeaveCriticalSection(&outputFiles[index].cs);
				if (recordNumber[index] == M) {
					PulseEvent(event);
					EnterCriticalSection(&outputUpdateCS);
					outputIndex = index;
					recordNumber[index] = 0;
				}
			}
		} while (FindNextFile(directory, &fileInfo));
		PulseEvent(event);
	}
	ExitThread(0);
}

DWORD WINAPI updateFunction(LPVOID arg) {
	HANDLE inputFile;
	OutputRecord rec;
	DWORD nRead;
	while (!stop) {
		WaitForSingleObject(event, INFINITE);
		EnterCriticalSection(&outputFiles[outputIndex].cs);
		inputFile = CreateFile(outputFiles[outputIndex].fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (inputFile == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, _T("Error %i when opening the outputFile %s"), GetLastError(), outputFiles[outputIndex].fileName);
			stop = TRUE;
			ExitThread(0);
		}
		SetFilePointer(inputFile, -(M * (INT)sizeof(OutputRecord)), NULL, FILE_END);
		_ftprintf(stdout, _T("------------------%s\n"), outputFiles[outputIndex].fileName);
		for (INT i = 0; i < M; i++) {
			ReadFile(inputFile, &rec, sizeof(OutputRecord), &nRead, NULL);
			_ftprintf(stdout, _T("%s %i %i\n"), rec.fileName, rec.charCount, rec.lineCount);
		}
		CloseHandle(inputFile);
		LeaveCriticalSection(&outputFiles[outputIndex].cs);
		LeaveCriticalSection(&outputUpdateCS);
	}
	ExitThread(0);
}

DWORD GetFileType(LPWIN32_FIND_DATA fileInfo) {
	BOOL isDir;
	DWORD fileType;
	isDir = ((fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
	fileType = TYPE_FILE;
	if (isDir) {
		if (_tcscmp(fileInfo->cFileName, _T(".")) == 0
			|| _tcscmp(fileInfo->cFileName, _T("..")) == 0) {
			fileType = TYPE_DOT;
		}
		else
			fileType = TYPE_DIR;
	}
	return fileType;
}