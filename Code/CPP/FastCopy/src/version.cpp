static char *version_id = 
	"@(#)Copyright (C) 20004-2010 H.Shirouzu	Version.cpp	ver2.03";
/* ========================================================================
	Project  Name			: Fast/Force copy file and directory
	Module Name				: Version
	Create					: 2010-06-13(Sun)
	Update					: 2010-09-12(Sun)
	Copyright				: H.Shirouzu
	Reference				: 
	======================================================================== */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tlib/tlib.h"
#include "version.h"

static char version_str[32];
static char copyright_str[128];

void SetVersionStr(BOOL is_runas)
{
	sprintf(version_str, "%.20s%.10s", strstr(version_id, "ver"),
			is_runas && TIsUserAnAdmin() ? " (Admin)" : "");
}

const char *GetVersionStr()
{
	if (version_str[0] == 0)
		SetVersionStr();
	return	version_str;
}

const char *GetCopyrightStr(void)
{
	if (copyright_str[0] == 0) {
		char *s = strchr(version_id, 'C');
		char *e = strchr(version_id, '\t');
		if (s && e && s < e) {
			sprintf(copyright_str, "%.*s", e-s, s);
		}
	}
	return	copyright_str;
}

