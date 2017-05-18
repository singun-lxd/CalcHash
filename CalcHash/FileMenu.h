#pragma once
#include <windef.h>

BOOL CheckMenuAvailable(HWND hWnd = NULL);
BOOL AddRightButtonMenu(HWND hWnd = NULL, BOOL bUINotify = TRUE);
BOOL RemoveRightButtonMenu(HWND hWnd = NULL, BOOL bUINotify = TRUE);
