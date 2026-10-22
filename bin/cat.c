#include <fileapi.h>
#include <stdio.h>
#include <windows.h>

#define BUFREADSIZE 4096

#define LOW_NUM_ARGS 2
#define OPENFILE_ERR 3
#define READ_ERR 4
#define INC_NUM_ARGS 5


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
    
    char *supposedFile = argv[1];
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