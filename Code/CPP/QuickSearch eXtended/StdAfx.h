// StdAfx.h: standard include files

#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#pragma warning(disable:4996)

#include <windows.h>
#include <stdlib.h>
#include <shellapi.h>

#include <fstream>     // debug output
#include <string>      // wstring and string
#include <vector>      // levenshtein
#include <map>         // replace chars / cache
#include <list>        // cache
#include <algorithm>   // lowercase
#include "deelx.h"     // RegEx

using namespace std;
