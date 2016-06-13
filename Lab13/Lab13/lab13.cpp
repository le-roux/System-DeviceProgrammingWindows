#include "lab13.h"

CRITICAL_SECTION inputFileCS, outputUpdateCS;
BOOLEAN stop = FALSE;
HANDLE event, backEvent;
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
		_ftprintf(stderr, _T("Wrong number of arguments\nUsage : %s S N M\n"), argv[0]);
		_ftscanf(stdin, _T("%c"), &a);
		return 1;
	}

	N = _tstoi(argv[2]);
	M = _tstoi(argv[3]);

	// Initialize the synchronisation objects
	InitializeCriticalSection(&inputFileCS);
	InitializeCriticalSection(&outputUpdateCS);
	event = CreateEvent(NULL, FALSE, FALSE, NULL);
	backEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	for (INT i = 0; i < MAX_LENGTH; i++)
		recordNumber[i] = 0;

	threadsHandles = (LPHANDLE)malloc((N + 1) * sizeof(HANDLE));
	threadsIds = (LPDWORD)malloc((N + 1) * sizeof(DWORD));

	// Create the explorer threads
	for (DWORD i = 0; i < N; i++) {
		threadsHandles[i] = CreateThread(NULL, 0, threadFunction, argv[1], 0, &threadsIds[i]);
		if (threadsHandles[i] == NULL) {
			for (DWORD j = 0; j <= i; j++) {
				CloseHandle(threadsHandles[j]);
			}
		}
	}

	// Create the output thread
	threadsHandles[N] = CreateThread(NULL, 0, updateFunction, NULL, 0, &threadsIds[N]);
	
	// Wait for all the explorer threads
	WaitForMultipleObjects(N, threadsHandles, TRUE, INFINITE);
	
	// Cleaning
	for (DWORD i = 0; i <= N; i++)
		CloseHandle(threadsHandles[i]);
	free(threadsHandles);
	free(threadsIds);

	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

DWORD WINAPI threadFunction(LPVOID arg) {
	HANDLE inputFile, directory, file;
	Record record;
	DWORD nIn, index = 0, fileCount = 0;
	WIN32_FIND_DATA fileInfo;
	TCHAR pattern[2 * MAX_LENGTH];
	LPTSTR inputFileName;
	inputFileName = (LPTSTR)arg;
	while (!stop) {
		BOOLEAN ret;

		// Read a record in the input file
		EnterCriticalSection(&inputFileCS);
		inputFile = CreateFile(inputFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (inputFile == INVALID_HANDLE_VALUE) {
			return 1;
		}
		SetFilePointer(inputFile, offset * sizeof(Record), NULL, FILE_BEGIN);
		ret = ReadFile(inputFile, &record, sizeof(Record), &nIn, NULL);
		
		// Find the index corresponding to the output file
		index = 0;
		while (outputFiles[index].valid) {
			if (_tcscmp(outputFiles[index].fileName, record.outputName) == 0)
				break;
			index++;
		}
		if (!outputFiles[index].valid) {
			outputFiles[index].valid = TRUE;
			_tcscpy(outputFiles[index].fileName, record.outputName);
			InitializeCriticalSection(&outputFiles[index].cs);
		}
		offset++;
		CloseHandle(inputFile);
		LeaveCriticalSection(&inputFileCS);
		if (sizeof(Record) != nIn) {
			stop = TRUE;
			ExitThread(1);
		}

		// Explore the matching files
		_tcscpy(pattern, record.directoryName);
		_tcscat(pattern, record.inputFileName);
		directory = FindFirstFile(pattern, &fileInfo);
		do {
			if (GetFileType(&fileInfo) == TYPE_FILE) {
				DWORD nRead, nWrite;
				OutputRecord rec;
				TCHAR filePath[2 * MAX_LENGTH];
				CHAR read;

				fileCount++;
				rec.fileCount = fileCount;
				rec.charCount = 0;
				rec.lineCount = 0;
				_tcscpy(rec.fileName, fileInfo.cFileName);
				_tcscpy(filePath, record.directoryName);
				_tcscat(filePath, fileInfo.cFileName);
				
				// Read the file
				file = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file == INVALID_HANDLE_VALUE) {
					_ftprintf(stderr, _T("Error %i when opening input file %s\n"), GetLastError(), filePath);
					ExitThread(1);
				}

				// Compute the statistics
				while (ReadFile(file, &read, sizeof(CHAR), &nRead, NULL) && nRead > 0) {
					rec.charCount++;
					if (read == '\n')
						rec.lineCount++;
				}

				CloseHandle(file);

				// Update the output file
				file = CreateFile(record.outputName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file == INVALID_HANDLE_VALUE) {
					_ftprintf(stderr, _T("Error %i when opening output file %s (index = %i)\n"), GetLastError(), record.outputName, index);
					ExitThread(1);
				}
				SetFilePointer(file, 0, NULL, FILE_END);
				WriteFile(file, &rec, sizeof(OutputRecord), &nWrite, NULL);
				CloseHandle(file);

				// Check if an output is necessary
				EnterCriticalSection(&outputFiles[index].cs);
				recordNumber[index]++;
				if (recordNumber[index] == M) {
					recordNumber[index] = 0;
					LeaveCriticalSection(&outputFiles[index].cs);
					
					// Signal the output thread
					EnterCriticalSection(&outputUpdateCS);
					outputIndex = index;
					SetEvent(event);
					WaitForSingleObject(backEvent, INFINITE);
				}
				else {
					LeaveCriticalSection(&outputFiles[index].cs);
				}
			}
		} while (FindNextFile(directory, &fileInfo));
		//PulseEvent(event);
	}
	ExitThread(0);
}

DWORD WINAPI updateFunction(LPVOID arg) {
	HANDLE inputFile;
	OutputRecord rec;
	DWORD nRead;

	while (!stop) {
		WaitForSingleObject(event, INFINITE);
		
		// Read the file
		inputFile = CreateFile(outputFiles[outputIndex].fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (inputFile == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, _T("Error %i when opening the outputFile %s"), GetLastError(), outputFiles[outputIndex].fileName);
			stop = TRUE;
			ExitThread(0);
		}
		SetFilePointer(inputFile, -(M * (INT)sizeof(OutputRecord)), NULL, FILE_END);
		_ftprintf(stdout, _T("------------------%s (index = %i)\n"), outputFiles[outputIndex].fileName, outputIndex);
		
		// Print the last M records
		for (INT i = 0; i < M; i++) {
			ReadFile(inputFile, &rec, sizeof(OutputRecord), &nRead, NULL);
			_ftprintf(stdout, _T("%i %s %i %i\n"), rec.fileCount, rec.fileName, rec.charCount, rec.lineCount);
		}
		CloseHandle(inputFile);
		PulseEvent(backEvent);
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