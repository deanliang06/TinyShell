#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fileapi.h>
#include <windows.h>

#define READBUF 1024

//check bin and then check cwd
int handleText(char* text) {
    //we do the following:
    //1. parse by spaces
    //2. try to find command if there is a (string).exe file in the dir (for rnow)
    //3. try to somehow make stdout a pipe that we write to this process (threads are harder bc separate executable
    //meaning no shared data/code, so a little odd ofr a choice)
    //3 (continued). we make our current stdout a write-end pipe and then
    //read from it

    HANDLE read, write = NULL;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    if (!CreatePipe(&read, &write, &sa, 0)) return 1;
    
    HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetStdHandle(STD_OUTPUT_HANDLE, write);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    //zero out the block of memory entirely
    SecureZeroMemory(&si, sizeof(si));
    SecureZeroMemory(&pi, sizeof(pi));

    (!CreateProcessA(
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
    ));

    SetStdHandle(STD_OUTPUT_HANDLE, outHandle);
    CloseHandle(outHandle);

    FILE *fd = fopen("text.txt", "w");
    char buf[READBUF];

    DWORD pCode = 0;
    GetExitCodeProcess(pi.hProcess, &pCode);
    char indication = 'a';
    PeekNamedPipe(read, &indication, 1, NULL, NULL, NULL);

    while (indication || pCode == STILL_ACTIVE) {
        DWORD numBytesRead = 0;
        ReadFile(read, buf, READBUF, &numBytesRead, NULL);
        fprintf(fd, "%s\n", buf);
        SecureZeroMemory(buf, READBUF);

        indication = 0;
        PeekNamedPipe(read, &indication, 1, NULL, NULL, NULL);
        GetExitCodeProcess(pi.hProcess, &pCode);
    }

    fclose(fd);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(read);
    CloseHandle(write);

}

