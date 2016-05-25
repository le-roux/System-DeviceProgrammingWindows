#include "ex1.h"

TCHAR a;
DWORD ret;
LPTSTR accountFileName;

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

	

	for (DWORD i = 0; i < threadsNb; i++) {
		threadsHandles[i] = CreateThread(NULL, 0, readOperations, argv[i + 2], 0, &threadsIds[i]);
		if (threadsHandles[i] == NULL) {
			_ftprintf(stderr, _T("Error %i when creating a thread\n"), GetLastError());
			_ftscanf(stdin, _T("%s"), &a);
			return 1;
		}
	}

	WaitForMultipleObjects(threadsNb, threadsHandles, TRUE, INFINITE);
	readAccountsFile(argv[1]);
	_ftscanf(stdin, _T("%c"), &a);
	return 0;
}

DWORD WINAPI readOperations(LPVOID arg) {
	LPTSTR fileName;
	HANDLE operationsFile, accountFile;
	Operation operation;
	DWORD nRead, nWrite;

	accountFile = CreateFile(accountFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (accountFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening the accounts file\n"), GetLastError());
		_ftscanf(stdin, _T("%s"), &a);
		return 1;
	}

	fileName = (LPTSTR)arg;
	operationsFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (operationsFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error %i when opening the operations file\n"), GetLastError());
		_ftscanf(stdin, _T("%s"), &a);
		return 1;
	}

	while (ReadFile(operationsFile, &operation, sizeof(Operation), &nRead, NULL) && nRead > 0) {
		LARGE_INTEGER nbOfBytesToLock;
		OVERLAPPED ov;
		Account account;
		BOOL ret;
		ov = { 0, 0, 0, 0, NULL };
		ov.Offset = (operation.lineNumber - 1) * sizeof(Account);
		nbOfBytesToLock.QuadPart = sizeof(Operation);
		LockFileEx(accountFile, LOCKFILE_EXCLUSIVE_LOCK, 0, nbOfBytesToLock.LowPart, nbOfBytesToLock.HighPart, &ov);
		__try {
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
			UnlockFileEx(accountFile, 0, nbOfBytesToLock.LowPart, nbOfBytesToLock.HighPart, &ov);
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

