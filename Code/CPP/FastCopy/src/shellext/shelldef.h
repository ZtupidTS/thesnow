/* static char *shelldef_id = 
	"@(#)Copyright (C) H.Shirouzu 2005   shelldef.h	Ver1.92"; */
/* ========================================================================
	Project  Name			: Fast/Force copy file and directory
	Create					: 2008-06-05(Thu)
	Update					: 2009-01-03(Sat)
	Copyright				: H.Shirouzu
	Reference				: 
	======================================================================== */

#ifndef SHELLDEF_H
#define SHELLDEF_H

#define FASTCOPY		"FastCopy"
#define SHELLEXT1_DLL	"FastCopy_shext.dll"
#define SHELLEXT2_DLL	"FastCopy_shext2.dll"
#define SHELLEXT3_DLL	"FastCopy_shext3.dll"
#define SHELLEXT4_DLL	"FastCopy_shext4.dll"
#define FCSHELLEXT1_DLL	"FastExt1.dll"
#define FCSHELLEX64_DLL	"FastEx64.dll"
#define FASTCOPY_EXE	"FastCopy.exe"
#define SHELLEXT_OPT	"/fc_shell_ext1"

#define MENU_FLAGS_OLD		"FastCopyMenuFlags"
#define MENU_FLAGS			"FastCopyMenuFlags2"
#define SHEXT_RIGHT_COPY	0x00000001
#define SHEXT_RIGHT_DELETE	0x00000002
#define SHEXT_RIGHT_PASTE	0x00000004
#define SHEXT_DD_COPY		0x00000010
#define SHEXT_DD_MOVE		0x00000020
#define SHEXT_SUBMENU_RIGHT	0x00001000
#define SHEXT_SUBMENU_DD	0x00002000
#define SHEXT_SUBMENU_NOSEP	0x00004000
#define SHEXT_ISSTOREOPT	0x00010000
#define SHEXT_NOCONFIRM		0x00020000
#define SHEXT_NOCONFIRMDEL	0x00040000
#define SHEXT_TASKTRAY		0x00080000
#define SHEXT_AUTOCLOSE		0x00100000
#define SHEXT_MENUFLG_EX	0x80000000
#define SHEXT_MENU_DEFAULT	0xfff00fff
#define SHEXT_MENU_NEWDEF	0xfff00fcc

#define SHEXT_RIGHT_MENUS	(SHEXT_RIGHT_COPY|SHEXT_RIGHT_DELETE|SHEXT_RIGHT_PASTE)
#define SHEXT_DD_MENUS		(SHEXT_DD_COPY|SHEXT_DD_MOVE)

#define SHEXT_MENU_COPY		0x00000000
#define SHEXT_MENU_DELETE	0x00000001
#define SHEXT_MENU_PASTE	0x00000002
#define SHEXT_MENU_MAX		0x00000003


#endif

