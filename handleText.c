#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fileapi.h>
#include <shlwapi.h>
#include <windows.h>
#include "memory.h"
#include "types.h"

#define READBUF 1024

char complete[READBUF];

int checkIfExecutable(char *path);
int checkIfCWD(char *line, char *buf, int start, int end);
void addExeToBuf(char *buf, char *text, int l, int r);
void getExe(char *fullLine, char *buf);
void changeCWD(char *text, ChatHistory *ch);
int checkIfCD(char *text, ChatHistory *ch);
int checkIfComplete(char *line, int l);

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

void addArgToBuf(char *buf, char *text) {
    //check if complete path
    //else attach cwd
    
    if (checkIfComplete(text, 0)) {
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

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    //zero out the block of memory entirely
    SecureZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdError = write;
    si.hStdOutput = write;
    
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
        CloseHandle(write);
        CloseHandle(read);
        char *err = "File execution error: ";
        printError(err, exe, ch);
        return;
    };

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

    memset(complete, 0, READBUF);
    while (text[startIdx] == ' ') startIdx++;;
    for (int i = startIdx; i < strlen(text) + 1; i++) {
        if (text[i] == ' ' || text[i] == '\0') {//comment
            addExeToBuf(complete, text, startIdx, i);
            startOfSArg = i+1;
            memcpy(complete+strlen(complete), text+i, strlen(text)-i + 1);
            break;      
        }
    }

    char first[READBUF];
    getExe(complete, first);//get executable to see if it is exe

    if (checkIfCD(first, ch)) {
        changeCWD(complete, ch);
    } else if (checkIfExecutable(first)) {
        executeFile(complete, first, ch);
    } else {
        char *msg = "Cannot find executable file named: ";
        printError(msg, first, ch);
    }
    return 0;
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
    memcpy(path+base+r-l, "", 1); //with null-term

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


void changeCWD(char *text, ChatHistory *ch) {
    char buf[READBUF];

    //get the first arg
    int startIdx = 0;
    int charGroup = 0;
    FILE *fd = fopen("text.txt", "w");

    int charGroups = 0;
    for (int i = 0; i < strlen(text) + 1; i++) {
        if (text[i] != ' ' && !charGroup) {
            charGroups++;
            charGroup = 1;
            if (charGroups == 2) {
                startIdx = i;
            }
        } else if (text[i] == ' ' && charGroup || text[i] == '\0' && charGroup) {
            if (charGroups == 2) {
                memcpy(buf, text+startIdx, i-startIdx);
                fprintf(fd, "Start: %d\nEnd: %d\n", startIdx, i);

                buf[i-startIdx] = '\0';
                break;
            } 
            charGroup = 0;
        }
    }

    char newBuf[READBUF];
    addArgToBuf(newBuf, buf);

    fprintf(fd, "%s\n", buf);
    fprintf(fd, "%s\n", newBuf);
    fprintf(fd, "%s\n", text);
    fclose(fd);
    

    if (!SetCurrentDirectoryA(newBuf)) {
        printError("This dir doesn't exist", buf, ch);
    }
}

int checkIfCD(char *text, ChatHistory *ch) {
    //get str for bin 
    char path[READBUF];
    int size = GetModuleFileNameA(NULL, path, READBUF);

    //null terminator
    size-=1;
    
    //find first backslash
    while (path[size] != '\\') size--;

    //add to after the first backslash and add \0 via string
    size++;
    memcpy(path+size, "bin\\", 5); //copy bin
    
    int newLen = strlen(path);
    memcpy(path+newLen, "cd.exe", 7);

    int same = 1;
    for (int i = 0; i < strlen(path) + 1; i++) {
        if (text[i] != path[i]) {
            same = 0;
        }
    }

    if (same) {
        return 1;
    }
    return 0;
}
