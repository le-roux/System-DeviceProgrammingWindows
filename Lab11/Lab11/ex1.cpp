#include "ex1.h"

//Garbage variable used as target for fscanf
TCHAR a;

//Name of the file containing the account infos
LPTSTR accountFileName;

/*Define one of the following symbol to get the
 corresponding synchronization mechanism : 
	- VERSION_A : fileLocking on a record-by-record basis
	- VERSION_B : critical section
	- VERSION_C : mutex
	- VERSION_D : semaphore*/

#define VERSION_D

#ifdef VERSION_B
	CRITICAL_SECTION cs;
#endif //VERSION_B
#ifdef VERSION_C
	HANDLE mutex;
#endif
#ifdef VERSION_D
	HANDLE semaphore;
#endif

INT _tmain(INT argc, LPTSTR argv[]) {
	HANDLE *threadsHandles;
	DWORD threadsNb, *threadsIds;

	if (argc < 2) {
		_ftprintf(stderr, _T("Wrong number of arguments.\nProper usage : %s AccountsFile OperationsFiles\n"), argv[0]);
		return 1;
	}

	accountFileName = argv[1];
	readAccountsFile(argv[1]);

	threadsNb = argc - 2;
	threadsHandles = (HANDLE*)malloc(threadsNb * sizeof(HANDLE));
	threadsIds = (DWORD*)malloc(threadsNb * sizeof(DWORD));
	//Initialization of the synchronization mechanism
	#ifdef VERSION_B
		InitializeCriticalSection(&cs);
	#endif //VERSION_B
	#ifdef VERSION_C
		mutex = CreateMutex(NULL, FALSE, NULL);
	#endif //VERSION_C
	#ifdef VERSION_D
		semaphore = CreateSemaphore(NULL, 1, 1, NULL);
	#endif //VERSION_D

	//Creation of the threads
	for (DWORD i = 0; i < threadsNb; i++) {
		threadsHandles[i] = CreateThread(NULL, 0, readOperations, argv[i + 2], 0, &threadsIds[i]);
		if (threadsHandles[i] == NULL) {
			_ftprintf(stderr, _T("Error %i when creating a thread\n"), GetLastError());
			_ftscanf(stdin, _T("%s"), &a);
			return 1;
		}
	}

	WaitForMultipleObjects(threadsNb, threadsHandles, TRUE, INFINITE);
	
	free(threadsHandles);
	threadsHandles = NULL;
	free(threadsIds);
	threadsIds = NULL;

	#ifdef VERSION_B
		DeleteCriticalSection(&cs);
	#endif //VERSION_B
	#ifdef VERSION_C
		CloseHandle(mutex);
	#endif //VERSION_C
	#ifdef VERSION_D
		CloseHandle(semaphore);
	#endif //VERSION_D
	readAccountsFile(argv[1]);
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

DWORD WINAPI readOperations(LPVOID FileName) {
	LPTSTR fileName;
	HANDLE operationsFile, accountFile;
	Operation operation;
	DWORD nRead, nWrite;

	//Open the file containing the account infos
	accountFile = CreateFile(accountFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (accountFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening the accounts file\n"), GetLastError());
		_ftscanf(stdin, _T("%s"), &a);
		return 1;
	}

	//Open the file containing the operations to apply
	fileName = (LPTSTR)FileName;
	operationsFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (operationsFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening the operations file\n"), GetLastError());
		_ftscanf(stdin, _T("%s"), &a);
		return 1;
	}

	//Read the operations
	while (ReadFile(operationsFile, &operation, sizeof(Operation), &nRead, NULL) && nRead > 0) {
		#ifdef VERSION_A
		LARGE_INTEGER nbOfBytesToLock;
		#endif //VERSION_A
		OVERLAPPED ov;
		Account account;
		BOOL ret;

		ov = { 0, 0, 0, 0, NULL };
		ov.Offset = (operation.lineNumber - 1) * sizeof(Account);
		
		#ifdef VERSION_A
			nbOfBytesToLock.QuadPart = sizeof(Operation);
			LockFileEx(accountFile, LOCKFILE_EXCLUSIVE_LOCK, 0, nbOfBytesToLock.LowPart, nbOfBytesToLock.HighPart, &ov);
		#endif //VERSION_A
		#ifdef VERSION_B
				EnterCriticalSection(&cs);
		#endif //VERSION_B
		#ifdef VERSION_C
				WaitForSingleObject(mutex, INFINITE);
		#endif //VERSION_C
		#ifdef VERSION_D
				WaitForSingleObject(semaphore, INFINITE);
		#endif //VERSION_D
		
		__try {
			//Apply the operation
			ret = ReadFile(accountFile, &account, sizeof(Account), &nRead, &ov);
			if (nRead != sizeof(Account) || !ret) {
				_ftprintf(stderr, _T("Error %i reading the account file\n"), GetLastError());
				continue;
			}

			account.balance += operation.operation;
			
			WriteFile(accountFile, &account, sizeof(Account), &nWrite, &ov);
			if (nWrite != sizeof(Account)) {
				_ftprintf(stderr, _T("Error %i writing the account file\n"), GetLastError());
				continue;
			}
		}
		__finally {
		#ifdef VERSION_A
			UnlockFileEx(accountFile, 0, nbOfBytesToLock.LowPart, nbOfBytesToLock.HighPart, &ov);
		#endif //VERSION_A
		#ifdef VERSION_B
			LeaveCriticalSection(&cs);
		#endif //VERSION_B
		#ifdef VERSION_C
			ReleaseMutex(mutex);
		#endif //VERSION_C
		#ifdef VERSION_D
			ReleaseSemaphore(semaphore, 1, NULL);
		#endif //VERSION_D
		}
	}

	CloseHandle(accountFile);
	CloseHandle(operationsFile);
	
	ExitThread(0);
}

VOID readAccountsFile(LPTSTR fileName) {
	HANDLE accountsFile;
	Account account;
	DWORD nRead;
	
	accountsFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (accountsFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening the accounts file\n"), GetLastError());
		return;
	}
	
	while (ReadFile(accountsFile, &account, sizeof(Account), &nRead, NULL) && nRead > 0) {
		_ftprintf(stdout, _T("%i %i %s %s %i\n"), account.lineNumber, account.bankAccountNumber, account.surname, account.name, account.balance);
	}
	_ftprintf(stdout, _T("\n"));
	
	CloseHandle(accountsFile);
}
