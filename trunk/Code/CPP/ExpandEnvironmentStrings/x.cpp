#include <windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
LPSTR lpCmdLine, int nCmdShow)
{
	MessageBox(NULL,"µ∞Ã€","",MB_OK);
	DWORD size=ExpandEnvironmentStrings("%windir%",NULL,260);
	CHAR* out=new CHAR[size+1];
	ExpandEnvironmentStringsA("%windir%",out,size);
	MessageBoxA(NULL,out,"fuck",MB_OK);
	return 0;
}