#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include "types.h"

int changeDisplay(HWND hwnd, WindowConfig *wConfig);
int initDeviceSettings(HWND hwnd);
int getInputSizing(HWND hwnd, WindowConfig *wConfig, RECT *toEdit, char *dir);
int resetTextBox(HWND hwnd, char *text);
int moveTextBox(HWND hwnd, WindowConfig *wc, char *dir);
