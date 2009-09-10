@echo off
call "c:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
cd win32
nmake -f scintilla.mak
pause
