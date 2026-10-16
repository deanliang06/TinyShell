#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "memory.h"

int freeHistory(ChatHistory *ch) {
    int len = ch->size;
    for (int i = 0; i < len; i++) {
        free(ch->messages[i].text);
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

    Message *toAdd = (Message *)malloc(sizeof(Message));
    if (!toAdd) return 0;

    toAdd->command = cmd;
    toAdd->text = text;
    toAdd->writing = writing;
    ch->messages[ch->size] = *toAdd; //really odd way but I think it works
    ch->size++;
    return 1;
}

//This is exclusively responsible for setting the text and doing next line with empty line
int completeCommand(ChatHistory *ch, char *text) {
    if (!editLastText(ch, text)) return 0;
    if (!addToHistory(ch, text, 0, 0)) return 0;
    return 1;
}
