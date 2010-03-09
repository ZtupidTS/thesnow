::ftp://ftp.info-zip.org/pub/infozip/win32/
::http://sourceforge.net/projects/infozip/files/

cd ..
del/q scite.zip
scite\zip.bin scite.zip scintilla\*.* scintilla\*\*.* scintilla\*\*\*.* scintilla\*\*\*\*.* scintilla\*\*\*\*\*.* scite\*.* scite\*\*.* scite\*\*\*.* scite\*\*\*\*.* -x *.o -x *.obj -x *.lib -x *.dll -x *.exe -x *.pdb -x *.res -x *.exp -x *.ncb -x *.sbr -x *.bsc -x *.ilk -x *.idb -x *.dpsession -x *.bak
cd scite
pause