#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <commctrl.h>
#include <processthreadsapi.h>
#include "memory.h"
#include "types.h"
#include "display.h"
#include "handleText.h"

#define WM_CREATECARET 0x8001
#define CWDBUF 512

typedef struct ShellArgs {
    HWND hwnd;
    char* text;
} ShellArgs;


static WindowConfig *wConfig;

int writingLastMessage(WindowConfig *wc) {
    int messageCount = wc->ch->size;
    return wc->ch->messages[messageCount-1].writing;
}

DWORD handleTextUpdate(LPVOID argConfig) {
    ShellArgs *shellArgs = (ShellArgs *) argConfig;
    handleText(shellArgs->text, wConfig->ch);//bro
    //Also must fetch CWD to get length to change EDIT Box
    char cwd[CWDBUF];
    getCWDPretty(cwd, CWDBUF);
    moveTextBox(shellArgs->hwnd, wConfig, cwd);

    //this just edits the messages etc
    editLastFull(wConfig->ch, "",1, 1);
    if (!InvalidateRect(GetParent(shellArgs->hwnd), NULL, TRUE)) ExitThread(1);

    //Show the caret 
    SendMessageA(shellArgs->hwnd, WM_CREATECARET, 0, 0);

    //prev malloced so we have to handle (up for debate boss for best practices)
    free(shellArgs);
    return 0;
}

LRESULT CALLBACK InputProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch(uMsg) {
        case WM_KEYDOWN:
            if (wParam == VK_RETURN && writingLastMessage(wConfig)) {
                int tLength = GetWindowTextLengthA(hwnd);
                char *text = (char *)malloc(sizeof(char) * (tLength + 1));
                if (!GetWindowTextA(hwnd, text, tLength + 1)) return 1;//what
                if (!completeCommand(wConfig->ch, text)) return 1;
                if (!HideCaret(hwnd)) return 1;
                if (!resetTextBox(hwnd, text)) return 1;
                if (!InvalidateRect(GetParent(hwnd), NULL, TRUE)) ExitThread(1);
                if (!InvalidateRect(hwnd, NULL, TRUE)) ExitThread(1);

                

                //create thread to execute the text handling
                ShellArgs *shellArg = (ShellArgs *)malloc(sizeof(ShellArgs));
                shellArg->hwnd = hwnd;
                shellArg->text = text;
                HANDLE thread = CreateThread(NULL, 0, handleTextUpdate, (LPVOID) shellArg, 0, NULL);
                CloseHandle(thread);
                return 0;
            } else if (!writingLastMessage(wConfig)) {
                return 0;
            }
        case WM_CREATECARET:
            ShowCaret(hwnd);
            return 0;
        default:
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_LBUTTONDOWN:
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            return 0;
        case WM_CREATE:
            return 0;
        case WM_PAINT:
            if (!changeDisplay(hwnd, wConfig)) return 1;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
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
    wConfig = &(WindowConfig){&chat, 0, 0, 0, 0, 12};
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
        NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);
    
    changeDisplay(hwnd, wConfig); //for the sole purpose of initing the wconfig
    InvalidateRect(hwnd, NULL, 1);//so that the display is repainted

    //intersante        
    RECT inputPlacement = {};

    char cwd[CWDBUF];
    getCWDPretty(cwd, CWDBUF);
    getInputSizing(hwnd, wConfig, &inputPlacement, cwd);

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

    SetWindowSubclass(textHW, InputProc, 1, 0);
    ShowWindow(textHW, nCmdShow);

    SetFocus(textHW);
    
    MSG msg = { };
    while (GetMessageA(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    freeHistory(wConfig->ch);
}