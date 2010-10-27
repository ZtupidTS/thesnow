/*static char *version_id = 
	"@(#)Copyright (C) 20004-2010 H.Shirouzu   version.cpp	ver2.02";
/* ========================================================================
	Project  Name			: Fast/Force copy file and directory
	Module Name				: Version
	Create					: 2010-06-13(Sun)
	Update					: 2010-06-13(Sun)
	Copyright				: H.Shirouzu
	Reference				: 
	======================================================================== */

#ifndef VERSION_H
#define VERSION_H

void SetVersionStr(BOOL is_runas=FALSE);
const char *GetVersionStr();
const char *GetCopyrightStr(void);

#endif

