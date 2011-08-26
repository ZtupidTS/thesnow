// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件:
#include <windows.h>
#include <stdio.h>  
#include <setupapi.h>  
#include <devguid.h>  
#include <regstr.h>  

#include <string>

#include "ras.h"
#include "raserror.h"
#pragma comment(lib, "rasapi32.lib")

// TODO: 在此处引用程序需要的其他头文件
