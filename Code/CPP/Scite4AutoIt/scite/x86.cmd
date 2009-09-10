@echo off
@call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
::@pushd ..\..\..\..\OftenUse\VC2008
::@call VC9.V6.x86.cmd
::@popd
@cd win32
nmake -f scite.mak
@pause
