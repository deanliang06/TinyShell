#include <stdio.h>
#include <fileapi.h>
#include <windows.h>
#include <errhandlingapi.h>

#define READBUF 1024

#define TOO_FEW_ARGS 1
#define TOO_MANY_ARGS 2
#define FILE_CREATION_ERR 3
#define EXISTS_ERR 4

void printError(int errNo) {
    if (errNo == TOO_FEW_ARGS) {
        printf("Need to provide a file to create");
    } else if (errNo == TOO_MANY_ARGS) {
        printf("Can only provide one file to create");
    } else if (errNo == FILE_CREATION_ERR) {
        printf("Error creating file");
    } else if (errNo == EXISTS_ERR) {
        printf("File already exists");
    }
}

int checkIfComplete(char *line) {
    char *compCheck = "C:\\";
    int completePath = 1;
    for (int i = 0; i < strlen(compCheck); i++) {
        if (compCheck[i] != line[i]) {
            completePath = 0;
            break;
        }
    }

    return completePath;
}

void addArgToBuf(char *buf, char *text) {
    //check if complete path
    //else attach cwd
    
    if (checkIfComplete(text)) {
        memcpy(buf, text, strlen(text) + 1);
        return;
    }

    char cwd[READBUF];

    GetCurrentDirectoryA(READBUF, cwd);
    
    //add back slash like a cat
    int toAdd = strlen(cwd);
    memcpy(cwd+toAdd, "\\", 2);

    int base = strlen(cwd);//redo
    memcpy(cwd+base, text, strlen(text) + 1);
    memcpy(buf, cwd, strlen(cwd) + 1);

    return;
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

    char supposedFile[READBUF];
    addArgToBuf(supposedFile, argv[1]);

    HANDLE handle = CreateFileA(
        supposedFile, 
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