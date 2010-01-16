// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/



// Declarations and definitions
// -------------

// --------------------
// Settings
// -------------
const int nFiles = 4;
const int StringSize = 5000; // Warning, mind this value, if exceeded a crash may occur

// --------------------
// Functions
// -------------
void StartFile(int width, int height, char* fname);
void StopFile();
int PrintFile(int a, char *fmt, ...);
void OpenConsole();
void CloseConsole();
#ifdef _WIN32
	HWND GetConsoleHwnd(void);
#endif


