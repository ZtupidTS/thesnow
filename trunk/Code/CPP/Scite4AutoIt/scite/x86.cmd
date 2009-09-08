@echo off
@pushd ..\..\..\..\OftenUse\VC2008
@call VC9.V6.x86.cmd
@popd
@cd win32
nmake -f scite.mak
@pause
