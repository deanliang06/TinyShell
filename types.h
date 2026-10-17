#ifndef TYPES_H
#define TYPES_H

typedef struct Message {
    int writing;
    char *text;
    char *dir;//hear me out it acts as cmd (NULL == normal txt)
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