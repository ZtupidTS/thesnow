README for building of Scintilla and SciTE

Scintilla 可以编译自己.
要编译SciTE,必须先编译Scintilla.


*** GTK+/Linux 版本 ***
不再支持
You must first have GTK+ 2.0 or later and GCC (3.1 or better) installed.
GTK+ 1.x will not work.
Other C++ compilers may work but may require tweaking the make file.

To build Scintilla, use the makefile located in the scintilla/gtk directory
	cd scintilla/gtk
	make
	cd ../..

To build and install SciTE, use the makefile located in the scite/gtk directory
	cd scite/gtk
	make
	make install

This installs SciTE into $prefix/bin. The value of $prefix is determined from
the location of Gnome if it is installed. This is usually /usr if installed
with Linux or /usr/local if built from source. If Gnome is not installed
/usr/bin is used as the prefix. The prefix can be overridden on the command
line like "make prefix=/opt" but the same value should be used for both make
and make install as this location is compiled into the executable. The global
properties file is installed at $prefix/share/scite/SciTEGlobal.properties.
The language specific properties files are also installed into this directory.

To remove SciTE
	make uninstall

To clean the object files which may be needed to change $prefix
	make clean

The current make file only supports static linking between SciTE and Scintilla.


*** Windows 版本 ***

你需要一个 C++ 编译器. Visual Studio 2008 为本软件的开发环境.
同样支持 Mingw32 3.1.但未经过测试.
For older versions of Borland C++ such as version 5.02, add the -l option.

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

*** GTK+/Windows version ***

Mingw32 is known to work. Other compilers will probably not work.

Only Scintilla will build with GTK+ on Windows. SciTE will not work.

To build Scintilla, make in the scintilla/gtk directory
	cd scintilla\gtk
	mingw32-make
