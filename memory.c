#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "memory.h"

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
    return 1;
}

int editLastFull(ChatHistory *ch, char *text, int cmd, int writing) {
    Message *last = &(ch->messages[ch->size-1]);
    FILE *fd = fopen("text.txt", "w");
    fprintf(fd, "We hate ourselves: %s, command: %d\n", last->text, last->command);//fuck windows
    fclose(fd);//rbuh


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
