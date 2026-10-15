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

typedef struct {
    ChatHistory *ch;
    int pHeight;
    int y;
    int pWidth;
    int x;
    int fontSize;
} WindowConfig;