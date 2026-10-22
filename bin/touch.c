#include <stdio.h>
#include <fileapi.h>
#include <windows.h>
#include <errhandlingapi.h>

#define TOO_FEW_ARGS 1
#define TOO_MANY_ARGS 2
#define FILE_CREATION_ERR 3
#define EXISTS_ERR 4

void printError(int errNo) {
    if (errNo == TOO_FEW_ARGS) {
        printf("Need to provide an file to create");
    } else if (errNo == TOO_MANY_ARGS) {
        printf("Can only provide one file to create");
    } else if (errNo == FILE_CREATION_ERR) {
        printf("Error creating file");
    } else if (errNo == EXISTS_ERR) {
        printf("File already exists");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printError(TOO_FEW_ARGS);
        return 0;
    } else if (argc > 2) {
        printError(TOO_MANY_ARGS);
        return 0;
    }

    SetLastError(0);

    HANDLE handle = CreateFileA(
        argv[1], 
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL, 
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE) {
        printError(FILE_CREATION_ERR);
    }

    if (handle && GetLastError() == ERROR_ALREADY_EXISTS) {
        printError(EXISTS_ERR);
    }

    CloseHandle(handle);
    return 0;
}