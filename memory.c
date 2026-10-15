#include <string.h>
#include <stdio.h>
#include "types.h"


int initHistory(ChatHistory *ch) {
    if (ch != NULL) return 0;

    
    Message *first = (Message *)malloc(sizeof(Message));
    if (!first) return 0;

    first->command = true;
    first->text = "";
    first->writing = true;
    
    ch->capacity = 1;
    ch->messages = first;
    ch->size = 1;
    return 1;
}

int addToHistory(ChatHistory *ch, char *text, bool cmd, bool writing) {
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

bool growhistory(ChatHistory *ch) {
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

int freeHistory(ChatHistory *ch) {
    int len = ch->size;
    for (int i = 0; i < len; i++) {
        if (!free(&(ch->messages[i]))) return 1;
    }
    return 0;
}