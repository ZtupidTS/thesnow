README for building of Scintilla and SciTE

Scintilla 可以编译自己.
要编译SciTE,必须先编译Scintilla.


*** GTK+/Linux 版本 ***
不再支持
*** Windows 版本 ***

你需要一个 C++ 编译器. Visual Studio 2008 为本软件的开发环境.
同样支持 Mingw32.但未经过测试.
Borland did work but the command lines are now too long and the most recent
Borland version does not work. For older versions such as version 5.02, add the -l option.

要编译 Scintilla, make in the scintilla/win32 directory
		cd scintilla\win32
GCC(not test):		mingw32-make
VS .NET:	nmake -f scintilla.mak
VC++ 6:	nmake -f scintilla_vc6.mak
Borland(not test): 	make -fscintilla.mak
		cd ..\..

要编译  SciTE, use the makefiles located in the scite/win32 directory
		cd scite\win32
GCC(not test):		mingw32-make
VS .NET: 	nmake -f scite.mak
Borland(not test):	make -fscite.mak

An executable SciTE will now be in scite\bin.

The Visual C++ 6.0 project (.dsp) files are no longer supported but are left
in the download for people that are prepared to update them.
