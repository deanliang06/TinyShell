#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "memory.h"

int freeHistory(ChatHistory *ch) {
    int len = ch->size;
    for (int i = 0; i < len; i++) {
        free(&(ch->messages[i]));
    }
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

    if (!freeHistory(ch)) return 0;
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
    return 1;
}

int addToHistory(ChatHistory *ch, char *text, int cmd, int writing) {
    if (ch->capacity == ch->size) {
        if (!growhistory(ch)) return 0;
    }

    Message *first = (Message *)malloc(sizeof(Message));
    if (!first) return 0;

    first->command = cmd;
    first->text = text;
    first->writing = writing;
    ch->messages[ch->size] = *first; //really odd way but I think it works
    ch->size++;
    return 1;
}
