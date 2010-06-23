@ECHO OFF
SET PATH=C:\MinGW32\bin;%path%

CD /D "%~dp0"
windres -o resfile.o toolbar.rc
IF ERRORLEVEL 1 EXIT

ld --strip-all --dll -o toolbar.dll resfile.o
REM gcc -s -shared -o toolbar.dll resfile.o
IF ERRORLEVEL 1 EXIT

DEL resfile.o
