#include "lab13.h"

CRITICAL_SECTION inputFileCS;
BOOLEAN stop = FALSE;
HANDLE event;

INT _tmain(INT argc, LPTSTR argv[]) {
	DWORD N, M;
	HANDLE inputFile;
	LPHANDLE threadsHandles;
	LPDWORD threadsIds;
	if (argc != 4) {
		return 1;
	}

	N = _tstoi(argv[2]);
	M = _tstoi(argv[3]);

	inputFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (inputFile == INVALID_HANDLE_VALUE) {
		return 1;
	}

	InitializeCriticalSection(&inputFileCS);

	threadsHandles = (LPHANDLE)malloc((N + 1) * sizeof(HANDLE));
	threadsIds = (LPDWORD)malloc((N + 1) * sizeof(DWORD));

	for (DWORD i = 0; i <= N; i++) {
		threadsHandles[i] = CreateThread(NULL, 0, threadFunction, &inputFile, 0, &threadsIds[i]);
	}
	WaitForMultipleObjects(N + 1, threadsHandles, TRUE, INFINITE);
	return 0;
}

DWORD WINAPI threadFunction(LPVOID arg) {
	HANDLE inputFile, directory, file;
	Record record;
	DWORD nIn;
	WIN32_FIND_DATA fileInfo;
	TCHAR pattern[2 * MAX_LENGTH];
	inputFile = *(LPHANDLE)arg;
	while (!stop) {
		EnterCriticalSection(&inputFileCS);
		ReadFile(inputFile, &record, sizeof(Record), &nIn, NULL);
		if (sizeof(Record) != nIn) {
			LeaveCriticalSection(&inputFileCS);
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
				TCHAR read;

				rec.charCount = 0;
				rec.lineCount = 0;
				_tcscpy(rec.fileName, fileInfo.cFileName);
				file = CreateFile(fileInfo.cFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file == INVALID_HANDLE_VALUE) {
					ExitThread(1);
				}
				while (ReadFile(file, &read, sizeof(TCHAR), &nRead, NULL) && nRead > 0) {
					rec.charCount++;
					if (read == '\n')
						rec.lineCount++;
				}
				CloseHandle(file);
				file = CreateFile(record.outputName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file == INVALID_HANDLE_VALUE) {
					ExitThread(1);
				}
				SetFilePointer(file, 0, NULL, FILE_END);
				WriteFile(file, &rec, sizeof(OutputRecord), &nWrite, NULL);
				CloseHandle(file);
			}
		} while (FindNextFile(directory, &fileInfo));
	}

}