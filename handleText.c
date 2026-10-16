#include <stdlib.h>
#include <unistd.h>
#include <windows.h>

void wait() {
    while(1) {}
}

int handleText(char *text) {
    WaitForSingleObject(wait, 10000);
    return 0;
}

