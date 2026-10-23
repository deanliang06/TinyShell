#include <stdio.h>
#include <windows.h>

#define TOO_FEW_A 1
#define TOO_MANY_A 2
#define LOC_ERR 3

#define BUFLEN 1024
#define READBUF 1024

void printError(int errCode) {
    if (errCode == TOO_FEW_A) {
        printf("Not enough arguments provided");
    } else if (errCode == TOO_MANY_A) {
        printf("Too many arguments provided. Only supporting 1 argument for ls.exe");
    } else if (errCode == LOC_ERR){
        printf("Error locating the directory");
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


    int base = strlen(cwd);
    memcpy(cwd+base, text, strlen(text) + 1);
    memcpy(buf, cwd, strlen(cwd) + 1);
    
    return;
}


int main(int argc, char *argv[]) {
    if (argc < 1) {
        printError(TOO_FEW_A);
        return 0;
    } else if (argc > 2) {
        printError(TOO_MANY_A);
        return 0;
    }

    char buf[BUFLEN];
    int cwdLen = GetCurrentDirectoryA(BUFLEN, buf);

    if (argc > 1) {
        addArgToBuf(buf, argv[1]);
    }

    //add wildcard
    int newLen = strlen(buf);
    memcpy(buf+newLen, "\\*", 3);

    SetLastError(0);
    WIN32_FIND_DATA findData = {};
    memset(&findData, 0, sizeof(WIN32_FIND_DATA));

    HANDLE handle = FindFirstFileA(buf, &findData);
    if (handle == INVALID_HANDLE_VALUE) {
        printError(LOC_ERR);
    }

    while (GetLastError() == 0) {
        if (strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, ".."))  {
            printf("%s\n", findData.cFileName);
        }
        FindNextFileA(handle, &findData);
    }

    FindClose(handle);
    return 0;
}