#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "memory.h"
#include "types.h"
#include "display.h"
#include "handleText.h"

static WindowConfig *GetAppState(HWND hwnd) {
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    WindowConfig *ch = (WindowConfig *)ptr;
    return ch;
}  

int writingLastMessage(WindowConfig *wc) {
    int messageCount = wc->ch->size;
    return wc->ch->messages[messageCount-1].writing;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_LBUTTONDOWN:
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            return 0;
        case WM_CREATE:
            CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
            WindowConfig *wConfig = (WindowConfig *)(cs->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wConfig);

            if (!initDeviceSettings(hwnd)) return 1;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            WindowConfig *wc = GetAppState(hwnd);
            int tLength = GetWindowTextLengthA(hwnd);
            char text[tLength];
            if (!GetWindowTextA(hwnd, text, tLength)) return 1;
            if (!completeCommand(wc, text)) return 1;
            if (!handleText(text)) return 1;
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "Sample Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    if (!RegisterClass(&wc)) return 1;

    ChatHistory chat = {};
    WindowConfig wConfig = {&chat, 0, 0, 0, 0, 12};
    if (!initHistory(&chat)) return 1;
    
    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Learn to Program Windows (Fine)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, 
        NULL,
        hInstance,
        &wConfig
    );

    if (hwnd == NULL)
    {
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);
    
    //init
    WindowConfig *wConfigP = GetAppState(hwnd);
    if (!changeDisplay(hwnd, wConfigP)) return 1;

    //if last message is currently being written then set up text box
    if (writingLastMessage(wConfigP)) {
        
        RECT inputPlacement = {};
        getInputSizing(wConfigP, &inputPlacement);

        // const char *file = "text.txt";
        // FILE *fd = fopen(file, "a");
        // if (!fd) {
        //     return 1;
        // }
        // fprintf(fd, "We are in loc x: %d, y: %d\n", inputPlacement.top, inputPlacement.left);
        // fclose(fd);

        HWND textHW = CreateWindowExA(
            0,
            "EDIT",
            NULL,
            ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD,
            inputPlacement.left, inputPlacement.top, inputPlacement.right-inputPlacement.left, inputPlacement.bottom-inputPlacement.top,//up for deebate
            hwnd, 
            NULL,
            hInstance,
            NULL
        );



        ShowWindow(textHW, nCmdShow);

        // RECT border;
        // if (!GetClientRect(textHW, &border)) return 1;

        // fd = fopen(file, "a");
        // if (!fd) {
        //     return 1;
        // }
        // fprintf(fd, "We are in loc x: %d, y: %d\n", border.top, border.left);
        // fclose(fd);

        SetFocus(textHW);
    }

    MSG msg = { };
    while (GetMessageA(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}