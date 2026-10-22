#include <stdio.h>
#include <fileapi.h>
#include <windows.h>
#include <errhandlingapi.h>

#define TOO_FEW_ARGS 1
#define TOO_MANY_ARGS 2
#define DIR_ERR 3
#define EXISTS_ERR 4
#define GEN_ERR 5

void printError(int errNo) {
    if (errNo == TOO_FEW_ARGS) {
        printf("Need to provide an file to create");
    } else if (errNo == TOO_MANY_ARGS) {
        printf("Can only provide one file to create");
    } else if (errNo == DIR_ERR) {
        printf("One or more of the intermediate directories in specified path don't exist");
    } else if (errNo == EXISTS_ERR) {
        printf("File already exists");
    } else if (errNo == GEN_ERR) {
        printf("Something went wrong");
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

    BOOL handle = CreateDirectoryA(
        argv[1], 
        NULL
    );

    if (!handle) {
        DWORD err = GetLastError();
        if (err == ERROR_ALREADY_EXISTS) {
            printError(EXISTS_ERR);
        } else if (err == ERROR_PATH_NOT_FOUND) {
            printError(DIR_ERR);
        } else {
            printError(GEN_ERR);
        }
    }

    return 0;
}