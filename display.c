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
            if (!getCWD(cwd, MAXBUFSIZE)) {
                ValidateRect(hwnd, NULL);
                ReleaseDC(hwnd, deviceContext);
                EndPaint(hwnd, &ps);
                return 0;
            }
            entry = strlen(cwd);
        }
        memcpy(&output[entry], ch->messages[i].text, strlen(ch->messages[i].text) + 1);
        if (!TextOutA(deviceContext, xConst, bottomY - curSpaceTaken,output,strlen(output))) {
            ValidateRect(hwnd, NULL);
            ReleaseDC(hwnd, deviceContext);
            EndPaint(hwnd, &ps);
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
int getInputSizing(HWND hwnd, WindowConfig *wConfig, RECT *toEdit, char *dir) {
    HDC hdc = GetDC(hwnd);
    SIZE textSize = {};
    GetTextExtentPoint32A(hdc, dir, strlen(dir), &textSize);
    LONG x = textSize.cx + 2 * OFFSET;


    toEdit->left = x;
    toEdit->top = wConfig->pHeight - OFFSET - LINESIZE;
    toEdit->bottom = wConfig->pHeight - OFFSET + LINESIZE;
    toEdit->right = wConfig->pWidth - OFFSET;
    ReleaseDC(hwnd, hdc);
    return 1;
}

int resetTextBox(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    FillRect(hdc, &ps.rcPaint, GetSysColorBrush(COLOR_WINDOW));
    SetWindowTextA(hwnd, "");
    SendMessageA(hwnd, EM_SETSEL, 0, 0);
    EndPaint(hwnd, &ps);
    ReleaseDC(hwnd, hdc);
    return 1;
}

int moveTextBox(HWND hwnd, WindowConfig *wc, char *dir) {
    RECT sizing = {};
    getInputSizing(hwnd, wc, &sizing, dir);

    //we can extract it from the getInputSizing (as we can make it general)
    int x = sizing.left;
    int y = sizing.top;
    int width = sizing.right-sizing.left;
    int height = sizing.bottom-sizing.top;

    //adjust the window with the text size
    MoveWindow(hwnd, x, y, width, height, TRUE);
    return 1;
}
