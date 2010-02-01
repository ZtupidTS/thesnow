#ifndef _EMUWINDOW_H
#define _EMUWINDOW_H

#include <windows.h>

namespace EmuWindow
{

HWND GetWnd();
HWND GetParentWnd();
HWND Create(HWND hParent, HINSTANCE hInstance, const TCHAR *title);
void Show();
void Close();
void SetSize(int displayWidth, int displayHeight);
void ToggleFullscreen(HWND hParent, bool bForceFull = false);
bool IsSizing();
void OSDMenu(WPARAM wParam);

}

#endif
