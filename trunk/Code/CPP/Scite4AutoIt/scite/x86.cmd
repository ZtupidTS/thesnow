@echo off
@call "%programfiles%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
@call "C:\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
::@pushd ..\..\..\..\OftenUse\VC2008
::@call VC9.V6.x86.cmd
::@popd
::VS2010
call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
@cd win32
nmake -f scite.mak
@pause
