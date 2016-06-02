/**
 * This program implement a simple communication mechanism
 * between 2 threads, by using a shared temp file.
 *
 * The second process runs the program ex3_2.exe, which must
 * be situated in the current directory.
 */
                                                                               
#include "ex3.h"

INT _tmain(INT argc, LPTSTR argv[]) {
    TCHAR fileName[MAX_PATH], commandLine[MAX_PATH + 2], buffer[MAX_LEN];
    HANDLE outputFile = NULL;
    DWORD nWritten;
    PROCESS_INFORMATION procInfo;
    STARTUPINFO startInfo;
    OVERLAPPED ov = { 0, 0, 0, 0, NULL }, ovN = { 0, 0, 0, 0, NULL };

    //Init
    size.QuadPart = MAX_LEN;
    GetStartupInfo(&startInfo);

    //Create the temp file used to communicate with the other process
    GetTempFileName(_T("./"), NULL, 0, fileName);
    outputFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ || FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (outputFile == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr, _T("Error %i when creating the temp file\n"), GetLastError());
        _ftscanf(stdin, _T("%c"), buffer);
        return 1;
    }


    __try {
        //Prepare a pseudo command line so that the actual argument is the second element
        commandLine[0] = _T('a');
        commandLine[1] = _T(' ');
        commandLine[2] = _T('\0');
        _tcscat(commandLine, fileName);

        LockFileEx(outputFile, LOCKFILE_EXCLUSIVE_LOCK, 0, size.LowPart, size.HighPart, &ov);

        CreateProcess(_T("./ex3_2.exe"), commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo);
        if (procInfo.hProcess == NULL) {
            _ftprintf(stderr, _T("Error %i when creating process\n"), GetLastError());
            __leave;
        }

        //Read strings from the console and write them in the temp file
        while(_tcscmp(buffer, END) != 0) {
            _ftscanf(stdin, _T("%s"), buffer);
            WriteFile(outputFile, buffer, MAX_LEN, &nWritten, &ov);
            //Lock the next record
            ovN.Offset += size.LowPart;
            LockFileEx(outputFile, LOCKFILE_EXCLUSIVE_LOCK, 0, size.LowPart, size.HighPart, &ovN);
            //Unlock the current record
            UnlockFileEx(outputFile, 0, size.LowPart, size.HighPart, &ov);
            ov.Offset += size.LowPart;
        }
    }
    __finally {
        CloseHandle(outputFile);
        DeleteFile(fileName);
        _ftscanf(stdin, _T("\n%c"), buffer);
    }
    return 0;
    
}