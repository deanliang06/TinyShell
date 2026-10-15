#ifndef TYPES_H
#define TYPES_H

typedef struct Message {
    int command;
    int writing;
    char* text;
} Message;

typedef struct ChatHistory {
    Message* messages;
    int capacity;
    int size;
} ChatHistory;

typedef struct WindowConfig {
    ChatHistory *ch;
    int pHeight;
    int y;
    int pWidth;
    int x;
    int fontSize;
} WindowConfig;

#endif