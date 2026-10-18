#include <windows.h>
#include <stdio.h>

int main(void) {
    volatile unsigned long count = 0;
    while (count < 1000000UL) ++count;
    printf("Count: %lu\n", count);
    fflush(stdout);
    /* Keep the child alive so the hosting shell can be sampled. */
    Sleep(2000);
    return count == 1000000UL ? 0 : 1;
}
