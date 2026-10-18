#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *scenario;
#if defined(WORKLOAD_OUTPUT)
    scenario = "output";
    /* Pace short lines so this tests history, rather than pipe saturation. */
    for (int i = 1; i <= 100; ++i) {
        printf("Line %03d\n", i);
        fflush(stdout);
        Sleep(25);
    }
#elif defined(WORKLOAD_MEMORY)
    scenario = "memory";
    const SIZE_T size = 64 * 1024 * 1024;
    volatile unsigned char *data = (volatile unsigned char *)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!data) return 2;
    for (SIZE_T i = 0; i < size; i += 4096) data[i] = 123;
    for (SIZE_T i = 0; i < size; i += 4096) if (data[i] != 123) return 3;
    puts("Touched and verified 64 MiB");
    fflush(stdout);
    Sleep(2000);
    VirtualFree((void *)data, 0, MEM_RELEASE);
#elif defined(WORKLOAD_REPEAT)
    scenario = "repeat";
    volatile unsigned long count = 0;
    while (count < 1000000UL) ++count;
    if (count != 1000000UL) return 4;
    printf("Count: %lu\n", count);
    fflush(stdout);
    Sleep(200);
#else
#error Select a workload
#endif
    const char *receipt = getenv("TINYSHELL_BENCH_RECEIPT");
    if (!receipt) return 5;
    FILE *file = fopen(receipt, "a");
    if (!file) return 6;
    fprintf(file, "%s,ok\n", scenario);
    return fclose(file) == 0 ? 0 : 7;
}
