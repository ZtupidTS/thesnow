# VirtuaWin (virtuawin.sourceforge.net)
# win32v6.mak - WinList make file for Microsoft MSVC v6.0
#
# Copyright (c) 2006-2010 VirtuaWin (VirtuaWin@home.se)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
#
##############################################################################

TOOLSDIR= c:\Program Files\Microsoft Visual Studio\vc98
CC      = cl
RC      = rc
LD	= link
CFLAGS	= -nologo -G5 -YX -GX -O2 -DNDEBUG "-I$(TOOLSDIR)\include"
CFLAGSD = -nologo -G5 -W3 -GX -Z7 -YX -Yd -Od -MLd "-I$(TOOLSDIR)\include"
LDFLAGS	= /SUBSYSTEM:windows /NOLOGO /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE "/LIBPATH:$(TOOLSDIR)\lib"
LDFLAGSD= /DEBUG /SUBSYSTEM:windows /NOLOGO /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE "/LIBPATH:$(TOOLSDIR)\lib"
LIBS	= shell32.lib user32.lib gdi32.lib comctl32.lib

!IFDEF vwUNICODE
CUCDEFS = -DUNICODE -D_UNICODE
!ENDIF

SRC	= Module.c

TARGET	= Module.exe
OBJS    = $(SRC:.c=.o)

TARGETD	= ModuleD.exe
OBJSD   = $(SRC:.c=.od)

.SUFFIXES: .rc .res .coff .c .o .od
.c.o:
	$(CC) $(CFLAGS) $(CUCDEFS) -c $< -Fo$@
.c.od:
	$(CC) $(CFLAGSD) $(CUCDEFS) -c $< -Fo$@

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) /out:$@ $(OBJS) $(OBJRES) $(LIBS)

$(TARGETD): $(OBJSD)
	$(LD) $(LDFLAGSD) /out:$@ $(OBJSD) $(OBJRES) $(LIBS)

all:    clean $(TARGET)

alld:   clean $(TARGETD)

clean: 
	- erase $(OBJS) $(OBJSD) vc60.pch

spotless: clean
	- erase $(TARGET) $(TARGETD)
