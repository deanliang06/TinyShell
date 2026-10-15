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

int addToHistory(ChatHistory *ch, char *text) {
    return 0;
}

int freeHistory(ChatHistory *ch) {
    return 0;
}