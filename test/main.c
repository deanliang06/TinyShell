#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Use two or more\n");
    }
    printf("%s\n", argv[1]);

    char *c = 0;
    printf("%c", *c);
}