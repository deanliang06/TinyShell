#include <string.h>
#include <stdio.h>
#include <synchapi.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include "types.h"
#include "memory.h"

#define BUFLINESIZE 512
#define OUTPUTBUFLEN 1024

typedef struct DirNav {
    char *cwd;
    char *cwdP;
    HANDLE mutex;
} DirNav;

typedef struct OutputBuf {
    char* buf;
    int size;
    HANDLE mutex;
} OutputBuf;

OutputBuf buf = {NULL, 0, NULL};
DirNav nav = {NULL, NULL, NULL};

int freeHistory(ChatHistory *ch) {
    int len = ch->size;
    for (int i = 0; i < len; i++) {
        free(ch->messages[i].text);
        if (ch->messages[i].dir) free(ch->messages[i].dir);
    }
    free(ch->messages);//bro
    return 0;
}

int growhistory(ChatHistory *ch) {
    int newSize = ch->capacity * 2;
    Message *newMessages = (Message *)malloc(newSize * sizeof(Message));
    if (!newMessages) return 0;

    Message *oMsgs = ch->messages;
    for (int i = 0; i < ch->size; i++) {
        memcpy(&newMessages[i], &oMsgs[i], sizeof(Message));
    }

    free(ch->messages);
    ch->messages = newMessages;
    ch->capacity = newSize;
    return 1;
}

int initHistory(ChatHistory *ch) {
    Message *first = (Message *)malloc(sizeof(Message));
    if (!first) return 0;

    char *cwd = (char *)malloc(sizeof(char) * BUFLINESIZE);
    getCWD(cwd, BUFLINESIZE);
    first->dir = cwd;
    first->text = "";
    first->writing = 1;
    
    ch->capacity = 1;
    ch->messages = first;
    ch->size = 1;

    //init nav mutex
    nav.mutex = OpenMutexA(SYNCHRONIZE, FALSE, "Directory changing");

    //init the outputbuffer vals
    buf.mutex = OpenMutexA(SYNCHRONIZE, FALSE, "Buffer Access");
    buf.buf = (char *)malloc(sizeof(char)* OUTPUTBUFLEN);

    return 1;
}

//used just to init next line as writing
int editLastFull(ChatHistory *ch, char *text, int cmd, int writing) {
    Message *last = &(ch->messages[ch->size-1]);

    //bc last line we can getcwd
    if (cmd) {
        char *dir = (char *)malloc(sizeof(char)*BUFLINESIZE);
        getCWD(dir, BUFLINESIZE);
        last->dir = dir;
    } else {
        last->dir = NULL;
    }
    last->text = text;
    last->writing = writing;

    return 1;
}

int editLastText(ChatHistory *ch, char *text) {
    Message *last = &(ch->messages[ch->size-1]);
    last->text = text;
    return 1;
}
//this is responsible for adding (stdout for executables 
//and subsequently adding the next line) only non-commands so far so NULL fine
int addToHistory(ChatHistory *ch, char *text, int writing) {
    if (ch->capacity == ch->size) {
        if (!growhistory(ch)) return 0;
    }

    Message *toAdd = &(ch->messages[ch->size]);
    if (!toAdd) return 0;

    toAdd->dir = NULL;
    toAdd->text = text;
    toAdd->writing = writing;
    ch->size++;
    return 1;
}


//This is exclusively responsible for setting the text and doing next line with empty line
//In cases when you submit a command/executable
int completeCommand(ChatHistory *ch, char *text) {
    if (!editLastText(ch, text)) return 0;
    if (!addToHistory(ch, "", 0)) return 0;
    return 1;
}

int getCWDPretty(char *buf, int bufLen) {
    findCWD();
    WaitForSingleObject(nav.mutex, INFINITE);
    memcpy(buf, nav.cwdP, min(bufLen, strlen(nav.cwdP) + 1));//fuck the memory leaks boss
    ReleaseMutex(nav.mutex);
    return 1;
}

int getCWD(char *buf, int bufLen) {
    findCWD();
    WaitForSingleObject(nav.mutex, INFINITE);
    memcpy(buf, nav.cwd, min(bufLen, strlen(nav.cwd) + 1));//fuck the memory leaks boss
    ReleaseMutex(nav.mutex);
    return 1;
}

void findCWD() {
    if (nav.cwd) free(nav.cwd);
    WaitForSingleObject(nav.mutex, INFINITE);
    char *newBuf = (char *)malloc(sizeof(char)*512);
    char *newBufP = (char *)malloc(sizeof(char)*512);
    
    //append the sideways caret
    GetCurrentDirectoryA(512, newBufP);
    memcpy(newBufP+strlen(newBufP), "> ", 3);
    nav.cwdP = newBufP;

    GetCurrentDirectoryA(512, newBuf);
    nav.cwd = newBuf;

    ReleaseMutex(nav.mutex);
}

void writeToOutputBuf(char *inBuf, ChatHistory *ch, DWORD numBytes) {
    WaitForSingleObject(buf.mutex, INFINITE);
    memcpy(buf.buf + buf.size, inBuf, numBytes);

    int lastStart = 0;
    int newBufSize = -1;
    buf.buf[buf.size+numBytes] = '\0';//buh

    for (int i = 0; i < buf.size+numBytes; i++) {
        if (buf.buf[i] == '\n') {
            char *temp = (char *)malloc(sizeof(char)*(i-lastStart+2));
            memcpy(temp, buf.buf+lastStart, i-lastStart+1);
            temp[i-lastStart+1] = '\0';
            editLastFull(ch, temp, 0, 0);
            addToHistory(ch, "", 0);
            lastStart = i+1;
        }

        if (i == buf.size+numBytes-1 && lastStart != i+1) {
            char temp[i-lastStart+1];
            memcpy(temp, buf.buf+lastStart, i-lastStart+1);//so no overwriting
            memcpy(buf.buf, temp, i-lastStart+1);
            newBufSize = i-lastStart+1;
        } else if (i == buf.size+numBytes-1) {
            newBufSize = 0;
        }
    }

    buf.size = newBufSize;
    ReleaseMutex(buf.mutex);
}
