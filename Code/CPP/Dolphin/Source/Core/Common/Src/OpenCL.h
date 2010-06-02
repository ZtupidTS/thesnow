// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#ifndef __OPENCL_H__
#define __OPENCL_H__

#include "Common.h"
// Change to #if 1 if you want to test OpenCL (and you have it) on Windows
#if 0
#pragma comment(lib, "OpenCL.lib")
#define HAVE_OPENCL 1
#endif

#if defined(HAVE_OPENCL) && HAVE_OPENCL

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#else

typedef void *cl_context;
typedef void *cl_command_queue;
typedef void *cl_program;
typedef void *cl_kernel;
typedef void *cl_mem;
typedef void *cl_int;

#endif

namespace OpenCL
{

#if defined(HAVE_OPENCL) && HAVE_OPENCL
extern cl_device_id device_id;
extern cl_context g_context;
extern cl_command_queue g_cmdq;
#endif

bool Initialize();

cl_context GetContext();

cl_command_queue GetCommandQueue();

void Destroy();

cl_program CompileProgram(const char *Kernel);
cl_kernel CompileKernel(cl_program program, const char *Function);

void HandleCLError(cl_int error, const char* str = 0);
}

#endif
