#ifndef EX1_H
#define EX1_h

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

typedef struct Account {
	INT lineNumber;
	INT bankAccountNumber;
	TCHAR surname[30];
	TCHAR name[30];
	INT balance;
} Account;

typedef struct Operation {
	INT lineNumber;
	INT bankAccountNumber;
	TCHAR surname[30];
	TCHAR name[30];
	INT operation;
} Operation;

/*Read a file containing operations and apply them on the
 corresponding account*/
DWORD WINAPI readOperations(LPVOID FileName);

/*Read a file containing accounts records and
 print its content on the standard output*/
VOID readAccountsFile(LPTSTR fileName);

#endif //EX1_H