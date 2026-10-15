#include <stdlib.h>
#include <unistd.h>

int handleText(char *text) {
    if (fork() != 0) {
        return 0;
    }
    while(true){

    }
    return 0;
}