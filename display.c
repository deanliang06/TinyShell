#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "types.h"

#define LINESIZE 24 // 12 px top
#define OFFSET 12 // 12 px from bottom and from left
#define MAXBUFSIZE 512 //for lines

int changeDisplay(HWND hwnd, WindowConfig *wConfig) {
    //just init for now (probably 1000 bugs)
    ChatHistory *ch = wConfig->ch;
    HDC deviceContext = GetDC(hwnd);
    const int bottomY = wConfig->y  + wConfig->pHeight - OFFSET;
    const int xConst = wConfig->x + OFFSET;

    const int chatSize = wConfig->ch->size;
    const int usableWindowHeight = wConfig->pHeight - OFFSET; // bc we padd the top by 12 we only bottom

    int curSpaceTaken = 0;

    for (int i = chatSize - 1; i >= 0 && curSpaceTaken < usableWindowHeight; i++) {
        char output[MAXBUFSIZE];
        int entry = 0;
        if (ch->messages[i].command) {
            memcpy(output, "cwd:", 5);
            entry = 4;
        }
        memcpy(output+entry, ch->messages[i].text, strlen(ch->messages[i].text));

        if (!TextOut(deviceContext, xConst, bottomY - curSpaceTaken,output,strlen(output))) {
            ReleaseDC(hwnd, deviceContext);
            return 0;
        }

        curSpaceTaken += LINESIZE;
    }

    ReleaseDC(hwnd, deviceContext);
    return 1;
}

int initDeviceSettings(HWND hwnd) {
    HDC deviceContext = GetDC(hwnd);
    SetTextAlign(deviceContext, TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
    ReleaseDC(hwnd, deviceContext);
}