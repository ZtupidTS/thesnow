@echo off
echo clean makefile...
del .\charset\sbcsdat.c
del .\mac\Makefile.mpw
del .\macosx\Makefile
del .\unix\Makefile.*
del .\windows\Makefile.*
rd /s /q .\windows\DEVCPP
rd /s /q .\windows\MSVC
echo cleaned.
pause