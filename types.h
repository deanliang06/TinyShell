typedef struct {
    bool command;
    bool writing;
    char* text;
} Message;

typedef struct {
    Message* messages;
    int capacity;
    int size;
} ChatHistory;