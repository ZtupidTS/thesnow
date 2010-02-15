::ftp://ftp.info-zip.org/pub/infozip/win32/
::http://sourceforge.net/projects/infozip/files/
@echo off
cd ..
del/q scintilla.zip
scite\zip scintilla.zip scintilla\*.* scintilla\*\*.* scintilla\*\*\*.* scintilla\*\*\*\*.* scintilla\*\*\*\*\*.* -x *.o -x *.obj -x *.dll -x *.lib -x *.res -x *.exp
cd scintilla
