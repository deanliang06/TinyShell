#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fileapi.h>
#include <shlwapi.h>
#include <windows.h>
#include "memory.h"
#include "types.h"

#define READBUF 1024
int dwordToHexStr(DWORD d, char *stuff);

void printError(char *errMsg, char* text, ChatHistory *ch) {
    char *persText = (char *)malloc(sizeof(char) * (strlen(text)+strlen(errMsg)) + 1);
    memcpy(persText, errMsg, strlen(errMsg));
    memcpy(persText +strlen(errMsg), text, strlen(text)+1);
    editLastFull(ch, persText, 0, 0);
    addToHistory(ch, "", 0);
}

void executeFile(char *text, ChatHistory *ch) {
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
        printError(err, text, ch);
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
        printError(sum, text, ch);
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

    //does path exist
    if (!PathFileExistsA(text)) {
        char *msg = "No file found under: ";
        printError(msg, text, ch);
        return 1;
    } 

    DWORD binaryType;
    if (!GetBinaryTypeA(text, &binaryType)) {
        char *msg = "Cannot execute file named: ";
        printError(msg, text, ch);
        return 1;
    }

    executeFile(text, ch);
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

