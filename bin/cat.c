#include <fileapi.h>
#include <stdio.h>
#include <windows.h>
#include <winbase.h>
#include <memory.h>

#define BUFREADSIZE 4096
#define READBUF 1024

#define LOW_NUM_ARGS 2
#define OPENFILE_ERR 3
#define READ_ERR 4
#define INC_NUM_ARGS 5

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

void printErrorCode(int errCode) {
    if (errCode == LOW_NUM_ARGS) {
        printf("Please provide file to read from\n");
    } else if(errCode == INC_NUM_ARGS) {
        printf("Please provide only one file to read from\n");
    } else if (errCode == OPENFILE_ERR) {
        printf("Error opening the file provided\n");
        printf("Check if it exists\n");
    } else if (errCode == READ_ERR) {
        printf("Error reading the file provided\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printErrorCode(LOW_NUM_ARGS);
        return 0;
    } else if (argc > 2) {
        printErrorCode(INC_NUM_ARGS);
        return 0;
    }
    

    char supposedFile[READBUF];
    addArgToBuf(supposedFile, argv[1]);

    HANDLE handle = CreateFileA(supposedFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE) {
        printErrorCode(OPENFILE_ERR);
        return 0;
    }

    char buf[BUFREADSIZE+1];
    DWORD numRead = 1;

    while (numRead) {
        ReadFile(handle, buf, BUFREADSIZE, &numRead, NULL);
        buf[numRead] = '\0';
        if (numRead) printf("%s", buf);
        memset(buf, 0, BUFREADSIZE);
    }

    CloseHandle(handle);
    return 0;
}

