#include "types.h"

int initHistory(ChatHistory *ch);
int addToHistory(ChatHistory *ch, char *text, int cmd, int writing);
int freeHistory(ChatHistory *ch);
int completeCommand(ChatHistory *ch, char *text);
int editLastFull(ChatHistory *ch, char *text, int cmd, int writing);


int getCWD(char *buf, int bufLen);
void findCWD();