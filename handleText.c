#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fileapi.h>
#include <shlwapi.h>
#include <windows.h>
#include "memory.h"
#include "types.h"

#define READBUF 1024

char complete[READBUF];

int dwordToHexStr(DWORD d, char *stuff);
int checkIfExecutable(char *path);
int checkIfComplete(char *line, int start);
int checkIfCWD(char *line, char *buf, int start, int end);
void addExeToBuf(char *buf, char *text, int l, int r);
void addArgToBuf(char *buf, char *text, int l, int r);
void getExe(char *fullLine, char *buf);



void printError(char *errMsg, char* text, ChatHistory *ch) {
    char *persText = (char *)malloc(sizeof(char) * (strlen(text)+strlen(errMsg)) + 1);
    memcpy(persText, errMsg, strlen(errMsg));
    memcpy(persText +strlen(errMsg), text, strlen(text)+1);
    editLastFull(ch, persText, 0, 0);
    addToHistory(ch, "", 0);
}

void executeFile(char *text, char *exe, ChatHistory *ch) {
    HANDLE read, write = NULL;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    if (!CreatePipe(&read, &write, &sa, 0)) return;
    
    HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE errHandle = GetStdHandle(STD_ERROR_HANDLE);

    SetStdHandle(STD_OUTPUT_HANDLE, write);
    SetStdHandle(STD_ERROR_HANDLE, write);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    //zero out the block of memory entirely
    SecureZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    
    SecureZeroMemory(&pi, sizeof(pi));

    

    if (!CreateProcessA(
        NULL,
        text,
        NULL,
        NULL,
        TRUE,
        NORMAL_PRIORITY_CLASS,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        SetStdHandle(STD_OUTPUT_HANDLE, outHandle);
        SetStdHandle(STD_ERROR_HANDLE, errHandle);
        CloseHandle(write);
        CloseHandle(read);
        char *err = "File execution error: ";
        printError(err, exe, ch);
        return;
    };

    SetStdHandle(STD_OUTPUT_HANDLE, outHandle);
    SetStdHandle(STD_ERROR_HANDLE, errHandle);

    CloseHandle(write);

    char buf[READBUF];

    DWORD numBytesRead;
    while (ReadFile(read, buf, sizeof(buf), &numBytesRead, NULL) && numBytesRead > 0) {
        //in memory (this is monitor)
        writeToOutputBuf(buf, ch, numBytesRead);
        for (int i = 0; i < READBUF; i++) buf[i] = 0; //zero out for reasons
    }

    flushOutputBuf(ch);

    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    if (code) {
        CloseHandle(read);

        char *err = "Exited execution with error code: 0x";
        char errBuf[32];
        int sizeHex = dwordToHexStr(code, errBuf);

        char sum[sizeHex + strlen(err) + 1];
        memcpy(sum, err, strlen(err));
        memcpy(sum+strlen(err), errBuf, sizeHex + 1);
        printError(sum, exe, ch);
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(read);
}

//check bin and then check cwd
int handleText(char* text, ChatHistory *ch) {
    //we do the following:
    //1. parse by spaces
    //2. try to find command if there is a (string).exe file in the dir (for rnow)
    //3. try to somehow make stdout a pipe that we write to this process (threads are harder bc separate executable
    //meaning no shared data/code, so a little odd ofr a choice)
    //3 (continued). we make our current stdout a write-end pipe and then
    //read from it

    //extract executable first and then arguments separately
    int startOfSArg = 0;
    int startIdx = 0;
    int firstArg = 1;

    memset(complete, 0, READBUF);
    while (text[startIdx] == ' ') startIdx++;;
    for (int i = startIdx; i < strlen(text); i++) {
        if (text[i] == ' ' && startOfSArg != i) {
            if (firstArg) {
                addExeToBuf(complete, text, startIdx, i);
                firstArg = 0;
                startOfSArg = i+1;
            } else {
                addArgToBuf(complete, text, startIdx, i);
                startOfSArg = i+1;
            }
        } else if (text[i] == ' ') {
            //adivce start of arg
            startOfSArg++;
        } else {
            //normal char
        }
    }

    if (!firstArg && startOfSArg != strlen(text)) {//burh
        addArgToBuf(complete, text, startOfSArg, strlen(text));//make again
        firstArg = 0;
    } else if (firstArg && startOfSArg != strlen(text)) {
        addExeToBuf(complete, text, startOfSArg, strlen(text));//make again
        firstArg = 0;
    }

    FILE *fd = fopen("text.txt", "w");
    fprintf(fd, "%s\n", complete);
    fclose(fd);
    
    char first[READBUF];
    getExe(complete, first);//get executable to seee if it is exe

    if (checkIfCD(first)) {
        changeCWD(complete);
    } else if (checkIfExecutable(first)) {
        executeFile(complete, first, ch);
    } else {
        char *msg = "Cannot find execute file named: ";
        printError(msg, first, ch);
    }
    return 0;
}

int dwordToHexStr(DWORD d, char *stuff) {
    DWORD dupl = d;
    int size = 0;
    while (dupl > 0) {
        dupl/=16;
        size++;
    }

    stuff[size] = '\0';
    int tmpSize = size;
    while (d > 0) {
        size--;
        if (d%16 >= 10) {
            stuff[size] = 'a' + (d%16-10);
        } else {
            stuff[size] = '0' + d%16;
        }
        d/=16;
    }
    return tmpSize;//excluding the null term
}

int checkIfExecutable(char *path) {
    if (!PathFileExistsA(path)) {
        return 0;
    } 

    DWORD binaryType;
    if (!GetBinaryTypeA(path, &binaryType)) {
        return 0;
    }
    return 1;
}

void addArgToBuf(char *buf, char *text, int l, int r) {
    //check if complete path
    //else attach cwd
    
    if (checkIfComplete(text, l)) {
        int base = strlen(buf);
        memcpy(buf+base, text+l, r-l);
        memcpy(buf+base+r-l, " ", 2); //with null-term
        return;
    }

    char cwd[READBUF];

    getCWD(cwd, READBUF);
    
    //add back slash like a cat
    int toAdd = strlen(cwd);
    memcpy(cwd+toAdd, "\\", 2);

    int base = strlen(cwd);
    memcpy(cwd+base, text+l, r-l);
    cwd[base+r-l] = '\0'; //with null-term

    base = strlen(buf);
    memcpy(buf+base, cwd, strlen(cwd));
    memcpy(buf+base+strlen(cwd), " ", 2); //with null-term
    return;
}

void addExeToBuf(char *buf, char *text, int l, int r) {
    //check if complete path then if not


    if (checkIfComplete(text, l)) {
        int base = strlen(buf);
        memcpy(buf+base, text+l, r-l);
        memcpy(buf+base+r-l, " ", 2); //with null-term
        return;
    }

    char cwd[READBUF];
    if (checkIfCWD(text, cwd, l, r)) {
        int base = strlen(buf);
        memcpy(buf+base, cwd, strlen(cwd));
        memcpy(buf+base+strlen(cwd), " ", 2); //with null-term
        return;
    }   

    //otherwise attach path bc most intensive operation
    //extract path
    char path[READBUF];
    int size = GetModuleFileNameA(NULL, path, READBUF);

    //null terminator
    size-=1;
    
    //find first backslash
    while (path[size] != '\\') size--;

    //add to after the first backslash and add \0 via string
    size++;
    memcpy(path+size, "bin\\", 5); //copy bin

    //add executable file name
    int base = strlen(path);
    memcpy(path+base, text+l, r-l);
    memcpy(path+base+r-l, " ", 2); //with null-term

    base = strlen(buf);
    memcpy(buf+base, path, strlen(path) + 1);
    return;
}

int checkIfComplete(char *line, int start) {
    char *compCheck = "C:\\";
    int completePath = 1;
    for (int i = 0; i < strlen(compCheck); i++) {
        if (compCheck[i] != line[start+i]) {
            completePath = 0;
            break;
        }
    }

    return completePath;
}

int checkIfCWD(char *line, char *cwd, int start, int end) {
    int inCWD = 0;
    getCWD(cwd, READBUF);
    
    //add back slash like a cat
    int toAdd = strlen(cwd);
    memcpy(cwd+toAdd, "\\", 2);

    int base = strlen(cwd);
    memcpy(cwd+base, line+start, end-start);
    memcpy(cwd+base+end-start, " ", 2); //with null-term

    if (checkIfExecutable(cwd)) {
        return 1;
    }
    return 0;
}

void getExe(char *completeLine, char *buf) {
    //assumes that complete line is null-terminated
    int len = strlen(completeLine);

    int startIdx = 0;
    int started = 0; 

    for (int i = 0; i < len+1; i++) {
        if (!started) {
            if (completeLine[i] != ' ') {
                started = 1;
            } else {
                startIdx = i+1;
            }
        } else if (completeLine[i] == ' ' || completeLine[i] == '\0') {
            memcpy(buf, completeLine+startIdx, i-startIdx);
            buf[i-startIdx] = '\0';
            return;
        }
    }
}


void changeCWD(char *text) {
    char buf[READBUF];

    //get the first arg
    int spaceGroups = 0;
    for (int i = 0; i < strlen(text)+1; i++) {

    }
    if (!SetCurrentDirectoryA(buf))
}
