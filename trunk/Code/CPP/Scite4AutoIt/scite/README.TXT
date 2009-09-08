README for building of Scintilla and SciTE

Scintilla 可以编译自己.
要编译SciTE,必须先编译Scintilla.

*** Windows 版本 ***

C++ compiler 必须为M$ VS2003 TO VS2008 OR VC++ 6.0

To build Scintilla, make in the scintilla/win32 directory
		cd scintilla\win32
VS .NET:	nmake -f scintilla.mak
VC++ 6:	nmake -f scintilla_vc6.mak


To build SciTE, use the makefiles located in the scite/win32 directory
		cd scite\win32
VS .NET: 	nmake -f scite.mak
Borland:	make -fscite.mak

An executable SciTE will now be in scite\bin.

The Visual C++ 6.0 project (.dsp) files are no longer supported but are left
in the download for people that are prepared to update them.
