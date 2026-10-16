#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "types.h"

#define LINESIZE 24 // 12 px top
#define OFFSET 2 // 12 px from bottom and from left
#define MAXBUFSIZE 512 //for lines

int changeDisplay(HWND hwnd, WindowConfig *wConfig) {
    //just init for now (probably 1000 bugs)
    HDC deviceContext = GetDC(hwnd);
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    FillRect(deviceContext, &ps.rcPaint, GetSysColorBrush(COLOR_WINDOW));

    ChatHistory *ch = wConfig->ch;
    
    //get client rect
    RECT border;
    if (!GetClientRect(hwnd, &border)) return 1;
    const int yHeight = border.bottom - border.top;
    const int xHeight = border.right - border.left;
    
    //get dims
    wConfig->pWidth = xHeight;
    wConfig->pHeight = yHeight;
    wConfig->x = border.left;
    wConfig->y = border.top;

    const int bottomY = 0 + yHeight - OFFSET - LINESIZE; //font-size means 12
    const int xConst = 0 + OFFSET;

    const int chatSize = wConfig->ch->size;
    const int usableWindowHeight = wConfig->pHeight - OFFSET; // bc we padd the top by 12 we only bottom

    int curSpaceTaken = 0;
    for (int i = chatSize - 1; i >= 0 && curSpaceTaken < usableWindowHeight; i--) {
        char output[MAXBUFSIZE];
        int entry = 0;
        if (ch->messages[i].command) {
            char cwd[MAXBUFSIZE];
            getCWD(cwd);
            memcpy(output, cwd, strlen(cwd));
            entry = 5;
        }
        memcpy(&output[entry], ch->messages[i].text, strlen(ch->messages[i].text) + 1);
        if (!TextOutA(deviceContext, xConst, bottomY - curSpaceTaken,output,strlen(output))) {
            ReleaseDC(hwnd, deviceContext);
            return 0;
        }
        curSpaceTaken += LINESIZE;
    }
    ValidateRect(hwnd, NULL);
    ReleaseDC(hwnd, deviceContext);
    EndPaint(hwnd, &ps);
    return 1;
}

int initDeviceSettings(HWND hwnd) {
    HDC deviceContext = GetDC(hwnd);
    SetTextAlign(deviceContext, TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
    ReleaseDC(hwnd, deviceContext);//bruh
}

//to Edit (to not be hardcoded)
int getInputSizing(WindowConfig *wConfig, RECT *toEdit) {
    toEdit->left = 32;
    toEdit->top = wConfig->pHeight - OFFSET - LINESIZE;//471-LINESIZE/2;
    toEdit->bottom = wConfig->pHeight - OFFSET + LINESIZE;
    toEdit->right = wConfig->pWidth - OFFSET;
    return 1;
}

int resetTextBox(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    FillRect(hdc, &ps.rcPaint, GetSysColorBrush(COLOR_WINDOW));
    SetWindowTextA(hwnd, "");
    SendMessageA(hwnd, EM_SETSEL, 0, 0);//
    EndPaint(hwnd, &ps);
    ReleaseDC(hwnd, hdc);
    return 1;
}
