#include <string.h>
#include <stdio.h>
#include <synchapi.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include "types.h"
#include "memory.h"

typedef struct DirNav {
    char *cwd;
    HANDLE mutex;
} DirNav;

DirNav nav = {NULL, NULL};

int freeHistory(ChatHistory *ch) {
    int len = ch->size;
    for (int i = 0; i < len; i++) {
        free(ch->messages[i].text);
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

    first->command = 1;
    first->text = "";
    first->writing = 1;
    
    ch->capacity = 1;
    ch->messages = first;
    ch->size = 1;

    nav.mutex = OpenMutexA(SYNCHRONIZE, FALSE, "Directory changing");

    return 1;
}

int editLastFull(ChatHistory *ch, char *text, int cmd, int writing) {
    Message *last = &(ch->messages[ch->size-1]);

    last->command = cmd;
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
//and subsequently adding the next line)
int addToHistory(ChatHistory *ch, char *text, int cmd, int writing) {
    if (ch->capacity == ch->size) {
        if (!growhistory(ch)) return 0;
    }

    Message *toAdd = &(ch->messages[ch->size]);
    if (!toAdd) return 0;

    toAdd->command = cmd;
    toAdd->text = text;
    toAdd->writing = writing;
    ch->size++;
    return 1;
}

//This is exclusively responsible for setting the text and doing next line with empty line
//In cases when you submit a command/executable
int completeCommand(ChatHistory *ch, char *text) {
    if (!editLastText(ch, text)) return 0;
    if (!addToHistory(ch, "", 0, 0)) return 0;
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
    GetCurrentDirectoryA(512, newBuf);
    nav.cwd = newBuf;
    ReleaseMutex(nav.mutex);
}
