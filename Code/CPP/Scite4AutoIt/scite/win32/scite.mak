﻿# Make file for SciTE on Windows Visual C++ version
# Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile is for using Visual C++ with nmake.
# Usage for Microsoft:
#     nmake -f scite.mak
# For debug versions define DEBUG on the command line.
# For a build without Lua, define NO_LUA on the command line.
# The main makefile uses mingw32 gcc and may be more current than this file.

.SUFFIXES: .cxx .properties

DIR_BIN=..\bin
PROPFILE=$(DIR_BIN)\属性文件
PROG=$(DIR_BIN)\SciTE.exe
PROGSTATIC=$(DIR_BIN)\Sc1.exe
DLLS=$(DIR_BIN)\Scintilla.dll $(DIR_BIN)\SciLexer.dll

WIDEFLAGS=-DUNICODE -D_UNICODE

CC=cl
RC=rc
LD=link

CXXFLAGS=-Zi -TP -MP -W4 -EHsc -Zc:forScope -Zc:wchar_t -D_CRT_SECURE_NO_DEPRECATE=1 -D_CRT_NONSTDC_NO_DEPRECATE $(WIDEFLAGS)
CCFLAGS=-TC -MP -W3 -wd4244 -D_CRT_SECURE_NO_DEPRECATE=1 -DLUA_USER_H=\"scite_lua_win.h\"

CXXDEBUG=-Od -MTd -DDEBUG
# Don't use "-MD", even with "-D_STATIC_CPPLIB" because it links to MSVCR71.DLL
CXXNDEBUG=-O1 -Oi -MT -DNDEBUG -GL
NAME=-Fo
LDFLAGS=-OPT:REF -LTCG -DEBUG
LDDEBUG=
LIBS=KERNEL32.lib USER32.lib GDI32.lib MSIMG32.lib COMDLG32.lib COMCTL32.lib ADVAPI32.lib IMM32.lib SHELL32.LIB OLE32.LIB UXTHEME.LIB
NOLOGO=-nologo

!IFDEF QUIET
CC=@$(CC)
CXXFLAGS=$(CXXFLAGS) $(NOLOGO)
CCFLAGS=$(CCFLAGS) $(NOLOGO)
LDFLAGS=$(LDFLAGS) $(NOLOGO)
!ENDIF

!IFDEF DEBUG
CXXFLAGS=$(CXXFLAGS) $(CXXDEBUG)
CCFLAGS=$(CCFLAGS) $(CXXDEBUG)
LDFLAGS=$(LDDEBUG) $(LDFLAGS)
!ELSE
CXXFLAGS=$(CXXFLAGS) $(CXXNDEBUG)
CCFLAGS=$(CCFLAGS) $(CXXNDEBUG)
!ENDIF

INCLUDEDIRS=-I../../scintilla/include -I../../scintilla/win32 -I../src

OBJS=\
	Extra.obj \
	SciTEBase.obj \
	FileWorker.obj \
	Cookie.obj \
	Credits.obj \
	FilePath.obj \
	JobQueue.obj \
	SciTEBuffers.obj \
	SciTEIO.obj \
	Exporters.obj \
	PropSetFile.obj \
	StringHelpers.obj \
	StringList.obj \
	SciTEProps.obj \
	Utf8_16.obj \
	SciTEWin.obj \
	SciTEWinBar.obj \
	SciTEWinDlg.obj \
	Strips.obj \
	IFaceTable.obj \
	DirectorExtension.obj \
	MultiplexExtension.obj \
	StyleDefinition.obj \
	StyleWriter.obj \
	GUIWin.obj \
	UniqueInstance.obj \
	WinMutex.obj

LEXLIB=..\..\scintilla\win32\Lexers.lib

OBJSSTATIC=\
	Extra.obj \
	SciTEBase.obj \
	FileWorker.obj \
	Cookie.obj \
	Credits.obj \
	FilePath.obj \
	JobQueue.obj \
	SciTEBuffers.obj \
	SciTEIO.obj \
	Exporters.obj \
	PropSetFile.obj \
	StringHelpers.obj \
	StringList.obj \
	SciTEProps.obj \
	Utf8_16.obj \
	Sc1.obj \
	SciTEWinBar.obj \
	SciTEWinDlg.obj \
	Strips.obj \
	IFaceTable.obj \
	DirectorExtension.obj \
	MultiplexExtension.obj \
	StyleDefinition.obj \
	StyleWriter.obj \
	GUIWin.obj \
	UniqueInstance.obj \
	WinMutex.obj \
	..\..\scintilla\win32\Accessor.obj \
	..\..\scintilla\win32\AutoComplete.obj \
	..\..\scintilla\win32\CallTip.obj \
	..\..\scintilla\win32\CaseConvert.obj \
	..\..\scintilla\win32\CaseFolder.obj \
	..\..\scintilla\win32\Catalogue.obj \
	..\..\scintilla\win32\CellBuffer.obj \
	..\..\scintilla\win32\CharacterCategory.obj \
	..\..\scintilla\win32\CharacterSet.obj \
	..\..\scintilla\win32\CharClassify.obj \
	..\..\scintilla\win32\ContractionState.obj \
	..\..\scintilla\win32\Decoration.obj \
	..\..\scintilla\win32\Document.obj \
	..\..\scintilla\win32\Editor.obj \
	..\..\scintilla\win32\ExternalLexer.obj \
	..\..\scintilla\win32\Indicator.obj \
	..\..\scintilla\win32\KeyMap.obj \
	..\..\scintilla\win32\LexerBase.obj \
	..\..\scintilla\win32\LexerModule.obj \
	..\..\scintilla\win32\LexerSimple.obj \
	..\..\scintilla\win32\LineMarker.obj \
	..\..\scintilla\win32\PerLine.obj \
	..\..\scintilla\win32\PlatWin.obj \
	..\..\scintilla\win32\PositionCache.obj \
	..\..\scintilla\win32\PropSetSimple.obj \
	..\..\scintilla\win32\RESearch.obj \
	..\..\scintilla\win32\RunStyles.obj \
	..\..\scintilla\win32\ScintillaBaseL.obj \
	..\..\scintilla\win32\ScintillaWinL.obj \
	..\..\scintilla\win32\Selection.obj \
	..\..\scintilla\win32\Style.obj \
	..\..\scintilla\win32\StyleContext.obj \
	..\..\scintilla\win32\UniConversion.obj \
	..\..\scintilla\win32\ViewStyle.obj \
	..\..\scintilla\win32\WordList.obj \
	..\..\scintilla\win32\XPM.obj

#++Autogenerated -- run ../scripts/RegenerateSource.py to regenerate
#**LEXPROPS=\\\n\($(DIR_BIN)\\\* \)
LEXPROPS=\
$(PROPFILE)\abaqus.properties $(PROPFILE)\ada.properties \
$(PROPFILE)\asl.properties $(PROPFILE)\asm.properties $(PROPFILE)\asn1.properties \
$(PROPFILE)\au3.properties $(PROPFILE)\ave.properties $(PROPFILE)\avs.properties \
$(PROPFILE)\baan.properties $(PROPFILE)\blitzbasic.properties \
$(PROPFILE)\bullant.properties $(PROPFILE)\caml.properties \
$(PROPFILE)\cmake.properties $(PROPFILE)\cobol.properties \
$(PROPFILE)\conf.properties $(PROPFILE)\cpp.properties \
$(PROPFILE)\csound.properties $(PROPFILE)\css.properties $(PROPFILE)\d.properties \
$(PROPFILE)\ecl.properties $(PROPFILE)\eiffel.properties \
$(PROPFILE)\erlang.properties $(PROPFILE)\escript.properties \
$(PROPFILE)\flagship.properties $(PROPFILE)\forth.properties \
$(PROPFILE)\fortran.properties $(PROPFILE)\freebasic.properties \
$(PROPFILE)\gap.properties $(PROPFILE)\haskell.properties \
$(PROPFILE)\html.properties $(PROPFILE)\inno.properties \
$(PROPFILE)\kix.properties $(PROPFILE)\latex.properties \
$(PROPFILE)\lisp.properties $(PROPFILE)\lot.properties \
$(PROPFILE)\lout.properties $(PROPFILE)\lua.properties \
$(PROPFILE)\matlab.properties $(PROPFILE)\metapost.properties \
$(PROPFILE)\mmixal.properties $(PROPFILE)\modula3.properties \
$(PROPFILE)\nimrod.properties $(PROPFILE)\nncrontab.properties \
$(PROPFILE)\nsis.properties $(PROPFILE)\opal.properties \
$(PROPFILE)\oscript.properties $(PROPFILE)\others.properties \
$(PROPFILE)\pascal.properties $(PROPFILE)\perl.properties \
$(PROPFILE)\pov.properties $(PROPFILE)\powerpro.properties \
$(PROPFILE)\powershell.properties $(PROPFILE)\ps.properties \
$(PROPFILE)\purebasic.properties $(PROPFILE)\python.properties \
$(PROPFILE)\r.properties $(PROPFILE)\rebol.properties $(PROPFILE)\ruby.properties \
$(PROPFILE)\rust.properties $(PROPFILE)\scriptol.properties \
$(PROPFILE)\smalltalk.properties $(PROPFILE)\sorcins.properties \
$(PROPFILE)\specman.properties $(PROPFILE)\spice.properties \
$(PROPFILE)\sql.properties $(PROPFILE)\tacl.properties $(PROPFILE)\tal.properties \
$(PROPFILE)\tcl.properties $(PROPFILE)\tex.properties \
$(PROPFILE)\txt2tags.properties $(PROPFILE)\vb.properties \
$(PROPFILE)\verilog.properties $(PROPFILE)\vhdl.properties \
$(PROPFILE)\yaml.properties
#--Autogenerated -- end of automatically generated section

PROPS=$(DIR_BIN)\全局设置.properties $(DIR_BIN)\全局缩写.properties $(LEXPROPS)

!IFNDEF NO_LUA
LUA_CORE_OBJS = lapi.obj lcode.obj ldebug.obj ldo.obj ldump.obj lfunc.obj lgc.obj llex.obj \
                lmem.obj lobject.obj lopcodes.obj lparser.obj lstate.obj lstring.obj \
                ltable.obj ltm.obj lundump.obj lvm.obj lzio.obj

LUA_LIB_OBJS =	lauxlib.obj lbaselib.obj ldblib.obj liolib.obj lmathlib.obj ltablib.obj \
                lstrlib.obj loadlib.obj loslib.obj linit.obj

LUA_OBJS = LuaExtension.obj $(LUA_CORE_OBJS) $(LUA_LIB_OBJS)

OBJS = $(OBJS) $(LUA_OBJS)
OBJSSTATIC = $(OBJSSTATIC) $(LUA_OBJS)
INCLUDEDIRS = $(INCLUDEDIRS) -I../lua/include
!ELSE
CXXFLAGS=$(CXXFLAGS) -DNO_LUA
!ENDIF

CXXFLAGS=$(CXXFLAGS) $(INCLUDEDIRS)
CCFLAGS=$(CCFLAGS) $(INCLUDEDIRS)


ALL: $(PROG) $(PROGSTATIC) $(DLLS) $(PROPS)

clean:
	del /q $(DIR_BIN)\*.exe *.o *.obj $(DIR_BIN)\*.dll *.res *.map $(DIR_BIN)\*.exp $(DIR_BIN)\*.lib $(DIR_BIN)\*.pdb

$(DIR_BIN)\Scintilla.dll: ..\..\scintilla\bin\Scintilla.dll
	copy ..\..\scintilla\bin\Scintilla.dll $@

$(DIR_BIN)\SciLexer.dll: ..\..\scintilla\bin\SciLexer.dll
	copy ..\..\scintilla\bin\SciLexer.dll $@

$(DIR_BIN)\SciTEGlobal.properties: ..\src\SciTEGlobal.properties
	copy ..\src\SciTEGlobal.properties $@

$(DIR_BIN)\abbrev.properties: ..\src\abbrev.properties
	copy ..\src\abbrev.properties $@
	
$(DIR_BIN)\全局设置.properties: ..\src\全局设置.properties
	copy ..\src\SciTEGlobal.properties $@
	
$(DIR_BIN)\全局缩写.properties: ..\src\全局缩写.properties
	copy ..\src\abbrev.properties $@

{..\src}.properties{$(PROPFILE)}.properties:
	copy $< $@

# Normally distributed rather than built as may not have grep on all machines
# Copy all non-comment lines from all the properties files into one combined file
..\src\Embedded.properties: $(PROPS)
	grep -v -h "^[#]" $(PROPS) >..\src\Embedded.properties

# A custom rule for .obj files built by scintilla:
..\..\scintilla\win32\PlatWin.obj: 	..\..\scintilla\win32\PlatWin.cxx
	@echo You must run the Scintilla makefile to build $*.obj
	@exit 255

SciTERes.res: SciTERes.rc ..\src\SciTE.h SciTE.exe.manifest
	$(RC) $(INCLUDEDIRS) -fo$@ SciTERes.rc

Sc1Res.res: SciTERes.rc ..\src\SciTE.h SciTE.exe.manifest
	$(RC) $(INCLUDEDIRS) -dSTATIC_BUILD -fo$@ SciTERes.rc

$(PROG): $(OBJS) SciTERes.res
	$(LD) $(LDFLAGS) -OUT:$@ $** $(LIBS)

$(PROGSTATIC): $(OBJSSTATIC) $(LEXLIB) Sc1Res.res
	$(LD) $(LDFLAGS) -OUT:$@ $** $(LIBS)

# Define how to build all the objects and what they depend on
# Some source files are compiled into more than one object because of different conditional compilation

{..\src}.cxx.obj::
	$(CC) $(CXXFLAGS) -c $<
{.}.cxx.obj::
	$(CC) $(CXXFLAGS) -c $<

{..\lua\src}.c.obj::
	$(CC) $(CCFLAGS) -c $<
{..\lua\src\lib}.c.obj::
	$(CC) $(CCFLAGS) -c $<

Sc1.obj: SciTEWin.cxx
	$(CC) $(CXXFLAGS) -DSTATIC_BUILD -c $(NAME)$@ SciTEWin.cxx

# Dependencies
DirectorExtension.obj: \
	DirectorExtension.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/Extender.h \
	DirectorExtension.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/SciTEBase.h
GUIWin.obj: \
	GUIWin.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h
SciTEWin.obj: \
	SciTEWin.cxx \
	SciTEWin.h \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h \
	../src/SciTEKeys.h \
	UniqueInstance.h \
	../src/StripDefinition.h \
	Strips.h \
	../src/MultiplexExtension.h \
	../src/Extender.h \
	DirectorExtension.h \
	../src/LuaExtension.h
Sc1.obj: \
	SciTEWin.cxx \
	SciTEWin.h \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h \
	../src/SciTEKeys.h \
	UniqueInstance.h \
	../src/StripDefinition.h \
	Strips.h \
	../src/MultiplexExtension.h \
	../src/Extender.h \
	DirectorExtension.h \
	../src/LuaExtension.h
SciTEWinBar.obj: \
	SciTEWinBar.cxx \
	SciTEWin.h \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h \
	../src/SciTEKeys.h \
	UniqueInstance.h \
	../src/StripDefinition.h \
	Strips.h
SciTEWinDlg.obj: \
	SciTEWinDlg.cxx \
	SciTEWin.h \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h \
	../src/SciTEKeys.h \
	UniqueInstance.h \
	../src/StripDefinition.h \
	Strips.h
Strips.obj: \
	Strips.cxx \
	SciTEWin.h \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h \
	../src/SciTEKeys.h \
	UniqueInstance.h \
	../src/StripDefinition.h \
	Strips.h
UniqueInstance.obj: \
	UniqueInstance.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	SciTEWin.h \
	../../scintilla/include/ILexer.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h \
	../src/SciTEKeys.h \
	UniqueInstance.h \
	../src/StripDefinition.h \
	Strips.h
WinMutex.obj: \
	WinMutex.cxx \
	../src/Mutex.h
Cookie.obj: \
	../src/Cookie.cxx \
	../src/SString.h \
	../src/Cookie.h
Credits.obj: \
	../src/Credits.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/SciTEBase.h
Exporters.obj: \
	../src/Exporters.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/SciTEBase.h
FilePath.obj: \
	../src/FilePath.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	../src/SString.h \
	../src/FilePath.h
FileWorker.obj: \
	../src/FileWorker.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/FilePath.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/Utf8_16.h
JobQueue.obj: \
	../src/JobQueue.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/PropSetFile.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h
MultiplexExtension.obj: \
	../src/MultiplexExtension.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	../src/MultiplexExtension.h \
	../src/Extender.h
PropSetFile.obj: \
	../src/PropSetFile.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/PropSetFile.h
SciTEBase.obj: \
	../src/SciTEBase.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/SciLexer.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h
Extra.obj: \
	../src/Extra.h\
	../src/Extra.cxx
SciTEBuffers.obj: \
	../src/SciTEBuffers.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/SciLexer.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h
SciTEIO.obj: \
	../src/SciTEIO.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/FileWorker.h \
	../src/SciTEBase.h \
	../src/Utf8_16.h
SciTEProps.obj: \
	../src/SciTEProps.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/SciLexer.h \
	../../scintilla/include/ILexer.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringList.h \
	../src/StringHelpers.h \
	../src/FilePath.h \
	../src/StyleDefinition.h \
	../src/PropSetFile.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/SciTE.h \
	../src/IFaceTable.h \
	../src/Mutex.h \
	../src/JobQueue.h \
	../src/Cookie.h \
	../src/Worker.h \
	../src/SciTEBase.h
StringHelpers.obj: \
	../src/StringHelpers.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	../src/StringHelpers.h
StringList.obj: \
	../src/StringList.cxx \
	../src/SString.h \
	../src/StringList.h
StyleDefinition.obj: \
	../src/StyleDefinition.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	../src/SString.h \
	../src/StringHelpers.h \
	../src/StyleDefinition.h
StyleWriter.obj: \
	../src/StyleWriter.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	../src/StyleWriter.h
Utf8_16.obj: \
	../src/Utf8_16.cxx \
	../src/Utf8_16.h

!IFNDEF NO_LUA
LuaExtension.obj: \
	../src/LuaExtension.cxx \
	../../scintilla/include/Scintilla.h \
	../src/GUI.h \
	../src/SString.h \
	../src/FilePath.h \
	../src/StyleWriter.h \
	../src/Extender.h \
	../src/LuaExtension.h \
	../src/IFaceTable.h \
	../src/SciTEKeys.h

IFaceTable.obj: \
	../src/IFaceTable.cxx \
	../src/IFaceTable.h

# Lua core dependencies are omitted; if the Lua source code
# is modified, a make clean may be necessary.
!ENDIF
