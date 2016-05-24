#ifndef UTILS_H
#define UTILS_H

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>

#define TYPE_FILE 0
#define TYPE_DIR 1
#define TYPE_DOT 2

/**
* A function that returns the type of the directory entry.
* Possible return values:
*		-TYPE_FILE
*		-TYPE_DIR
*		-TYPE_DOT
*/
DWORD FileType(LPWIN32_FIND_DATA fileInfo);

/**
* This function adds a slash add the end of the string if there is no
* and returns the (updated) string.
*/
LPTSTR addFinalSlash(LPTSTR input);

#endif //UTILS_H