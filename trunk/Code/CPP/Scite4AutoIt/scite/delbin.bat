del /S /Q *.a *.aps *.bsc *.dll *.dsw *.exe *.idb *.ilc *.ild *.ilf *.ilk *.ils *.lib *.map *.ncb *.obj *.o *.opt *.pdb *.plg *.res *.sbr *.tds *.exp >NUL:
rd /s /q bin
md bin\properties
md bin\�����ļ�
echo This empty files ensures that the directory is created. >bin\empty.txt
copy USkin.dl_ bin\USkin.dll /y
copy USkin.li_ win32\USkin.lib /y