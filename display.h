#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include "types.h"

int changeDisplay(HWND hwnd, WindowConfig *wConfig);
int initDeviceSettings(HWND hwnd);
int getInputSizing(WindowConfig *wConfig, RECT *toEdit);
int resetTextBox(HWND hwnd, char *text);
