#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "memory.h"
#include "types.h"
#include "display.h"

inline WindowConfig *GetAppState(HWND hwnd) {
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    WindowConfig *ch = (WindowConfig *)ptr;
    return ch;
}  

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_LBUTTONDOWN:
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            const char *file = "text.txt";
            FILE *fd = fopen(file, "a");
            if (!fd) {
                return 1;
            }
            fprintf(fd, "We are in loc x: %d, y: %d\n", xPos, yPos);
            fclose(fd);
            return 0;
        case WM_CREATE:
            CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
            WindowConfig *wConfig = (WindowConfig *)(cs->lpCreateParams);
            wConfig->pHeight = cs->cy;
            wConfig->y = cs->y;
            wConfig->pWidth = cs->cx;
            wConfig->x = cs->x;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wConfig);

            if (!initDeviceSettings(hwnd)) return 1;
            if (!changeDisplay(hwnd, wConfig)) return 1;

            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
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
    
    HWND hwnd = CreateWindowEx(
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
    
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}