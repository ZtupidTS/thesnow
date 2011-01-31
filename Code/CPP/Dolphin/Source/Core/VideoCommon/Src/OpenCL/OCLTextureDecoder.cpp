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

#include "OCLTextureDecoder.h"

#include "../OpenCL.h"
#include "FileUtil.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <string>

//#define DEBUG_OPENCL

cl_program g_program;

struct sDecoderParameter
{
	const char *name;
	cl_kernel kernel;
	float sizeOfSrc;
	float sizeOfDst;
	int xSkip;
	int ySkip;
	PC_TexFormat format;
};

sDecoderParameter g_DecodeParametersNative[] = {
	/* GX_TF_I4     */ { "DecodeI4",     NULL, 0.5f, 1, 8, 8, PC_TEX_FMT_I4_AS_I8 },
	/* GX_TF_I8     */ { "DecodeI8",     NULL,    1, 1, 8, 4, PC_TEX_FMT_I8 },
	/* GX_TF_IA4    */ { "DecodeIA4",    NULL,    1, 2, 8, 4, PC_TEX_FMT_IA4_AS_IA8 },
	/* GX_TF_IA8    */ { "DecodeIA8",    NULL,    2, 2, 4, 4, PC_TEX_FMT_IA8 },
	/* GX_TF_RGB565 */ { "DecodeRGB565", NULL,    2, 2, 4, 4, PC_TEX_FMT_RGB565 },
	/* GX_TF_RGB5A3 */ { "DecodeRGB5A3", NULL,    2, 4, 4, 4, PC_TEX_FMT_BGRA32 },
	/* GX_TF_RGBA8  */ { "DecodeRGBA8",  NULL,    4, 4, 4, 4, PC_TEX_FMT_BGRA32 },
	/* 7            */ { NULL },
	/* GX_TF_C4     */ { NULL },
	/* GX_TF_C8     */ { NULL },
	/* GX_TF_C14X2  */ { NULL },
	/* B            */ { NULL },
	/* C            */ { NULL },
	/* D            */ { NULL },
	/* GX_TF_CMPR   */ { "DecodeCMPR",   NULL, 0.5f, 4, 8, 8, PC_TEX_FMT_BGRA32 },
};

sDecoderParameter g_DecodeParametersRGBA[] = {
	/* GX_TF_I4     */ { "DecodeI4_RGBA",     NULL, 0.5f, 4, 8, 8, PC_TEX_FMT_RGBA32 },
	/* GX_TF_I8     */ { "DecodeI8_RGBA",     NULL,    1, 4, 8, 4, PC_TEX_FMT_RGBA32 },
	/* GX_TF_IA4    */ { "DecodeIA4_RGBA",    NULL,    1, 4, 8, 4, PC_TEX_FMT_RGBA32 },
	/* GX_TF_IA8    */ { "DecodeIA8_RGBA",    NULL,    2, 4, 4, 4, PC_TEX_FMT_RGBA32 },
	/* GX_TF_RGB565 */ { "DecodeRGB565_RGBA", NULL,    2, 4, 4, 4, PC_TEX_FMT_RGBA32 },
	/* GX_TF_RGB5A3 */ { "DecodeRGB5A3_RGBA", NULL,    2, 4, 4, 4, PC_TEX_FMT_RGBA32 },
	/* GX_TF_RGBA8  */ { "DecodeRGBA8_RGBA",  NULL,    4, 4, 4, 4, PC_TEX_FMT_RGBA32 },
	/* 7            */ { NULL },
	/* GX_TF_C4     */ { NULL },
	/* GX_TF_C8     */ { NULL },
	/* GX_TF_C14X2  */ { NULL },
	/* B            */ { NULL },
	/* C            */ { NULL },
	/* D            */ { NULL },
	/* GX_TF_CMPR   */ { "DecodeCMPR_RGBA",   NULL, 0.5f, 4, 8, 8, PC_TEX_FMT_RGBA32 },
};

bool g_Inited = false;
cl_mem g_clsrc, g_cldst;                    // texture buffer memory objects

#define HEADER_SIZE	32

void TexDecoder_OpenCL_Initialize()
{
	if(!g_Inited)
	{
		if(!OpenCL::Initialize())
			return;

		cl_int err = 1;
		size_t binary_size = 0;
		char *binary = NULL;
		char *header = NULL;
		size_t nDevices = 0;
		cl_device_id *devices = NULL;
		size_t *binary_sizes = NULL;
		char **binaries = NULL;		
		char filename[1024];
		char dolphin_rev[HEADER_SIZE];

		sprintf(filename, "%skernel.bin", File::GetUserPath(D_OPENCL_IDX));
		snprintf(dolphin_rev, HEADER_SIZE, "%-31s", svn_rev_str);

		FILE *input = NULL;

		input = fopen(filename, "rb");
		if (input == NULL)
		{
			binary_size = 0;
		}
		else
		{
			binary_size = File::GetSize(input);
			header = new char[HEADER_SIZE];
			binary = new char[binary_size];
			fread(header, sizeof(char), HEADER_SIZE, input);
			binary_size = fread(binary, sizeof(char),
				binary_size - HEADER_SIZE, input);
			fclose(input);
		}

		if (binary_size > 0)
		{
			if (binary_size > HEADER_SIZE)
			{
				if (strncmp(header, dolphin_rev, HEADER_SIZE) == 0)
				{
					g_program = clCreateProgramWithBinary(OpenCL::GetContext(), 1, &OpenCL::device_id, &binary_size, (const unsigned char**)&binary, NULL, &err);
					if (err != CL_SUCCESS)
					{
						OpenCL::HandleCLError(err, "clCreateProgramWithBinary");
					}
				
					if (!err)
					{
						err = clBuildProgram(g_program, 1, &OpenCL::device_id, NULL, NULL, NULL);
						if (err != CL_SUCCESS)
						{
							OpenCL::HandleCLError(err, "clBuildProgram");
						}
					}
				}
			}
			delete header;
			delete binary;
		}

		if (err)
		{
			std::string code;
			sprintf(filename, "%sTextureDecoder.cl", File::GetUserPath(D_OPENCL_IDX));
			if (!File::ReadFileToString(true, filename, code))
			{
				ERROR_LOG(VIDEO, "Failed to load OpenCL code %s - file is missing?", filename);
				return;
			}

			g_program = OpenCL::CompileProgram(code.c_str());

			err = clGetProgramInfo(g_program, CL_PROGRAM_NUM_DEVICES, sizeof(nDevices), &nDevices, NULL);
			if (err != CL_SUCCESS)
			{
				OpenCL::HandleCLError(err, "clGetProgramInfo");
			}
			devices = (cl_device_id *)malloc( sizeof(cl_device_id) *nDevices);

			err = clGetProgramInfo(g_program, CL_PROGRAM_DEVICES, sizeof(cl_device_id)*nDevices, devices, NULL);
			if (err != CL_SUCCESS)
			{
				OpenCL::HandleCLError(err, "clGetProgramInfo");
			}

			binary_sizes = (size_t *)malloc(sizeof(size_t)*nDevices);
			err = clGetProgramInfo(g_program, CL_PROGRAM_BINARY_SIZES,	sizeof(size_t)*nDevices, binary_sizes, NULL);
			if (err != CL_SUCCESS)
			{
				OpenCL::HandleCLError(err, "clGetProgramInfo");
			}

			binaries = (char **)malloc(sizeof(char *)*nDevices);
			for (u32 i = 0; i < nDevices; ++i)
			{
				if( binary_sizes[i] != 0 )
				{
					binaries[i] = (char *)malloc(HEADER_SIZE + binary_sizes[i]);
				}
				else
				{
					binaries[i] = NULL;
				}
			}
			err = clGetProgramInfo( g_program, CL_PROGRAM_BINARIES,	sizeof(char *)*nDevices, binaries, NULL );
			if (err != CL_SUCCESS)
			{
				OpenCL::HandleCLError(err, "clGetProgramInfo");
			}

			if (!err)
			{
				sprintf(filename, "%skernel.bin", File::GetUserPath(D_OPENCL_IDX));
				FILE *output = NULL;
				output = fopen(filename, "wb");

				if (output == NULL)
				{
					binary_size = 0;
				}
				else
				{
					// Supporting one OpenCL device for now
					fwrite(dolphin_rev, sizeof(char), HEADER_SIZE, output);
					fwrite(binaries[0], sizeof(char), binary_sizes[0], output);
					fclose(output);
				}
			}
		}

		for (int i = 0; i <= GX_TF_CMPR; ++i) {
			if (g_DecodeParametersNative[i].name)
				g_DecodeParametersNative[i].kernel =
				OpenCL::CompileKernel(g_program,
				g_DecodeParametersNative[i].name);

			if (g_DecodeParametersRGBA[i].name)
				g_DecodeParametersRGBA[i].kernel =
				OpenCL::CompileKernel(g_program,
				g_DecodeParametersRGBA[i].name);
		}

		// Allocating maximal Wii texture size in advance, so that we don't have to allocate/deallocate per texture
#ifndef DEBUG_OPENCL
		g_clsrc = clCreateBuffer(OpenCL::GetContext(), CL_MEM_READ_ONLY , 1024 * 1024 * sizeof(u32), NULL, NULL);
		g_cldst = clCreateBuffer(OpenCL::GetContext(), CL_MEM_WRITE_ONLY, 1024 * 1024 * sizeof(u32), NULL, NULL);
#endif

		g_Inited = true;
	}
}

void TexDecoder_OpenCL_Shutdown()
{
	if (g_program)
		clReleaseProgram(g_program);

	for (int i = 0; i < GX_TF_CMPR; ++i) {
		if (g_DecodeParametersNative[i].kernel)
			clReleaseKernel(g_DecodeParametersNative[i].kernel);

		if(g_DecodeParametersRGBA[i].kernel)
			clReleaseKernel(g_DecodeParametersRGBA[i].kernel);
	}

	if(g_clsrc)
		clReleaseMemObject(g_clsrc);

	if(g_cldst)
		clReleaseMemObject(g_cldst);

	g_Inited = false;
}

PC_TexFormat TexDecoder_Decode_OpenCL(u8 *dst, const u8 *src, int width, int height, int texformat, int tlutaddr, int tlutfmt, bool rgba)
{
	cl_int err;
	sDecoderParameter& decoder = rgba ? g_DecodeParametersRGBA[texformat] : g_DecodeParametersNative[texformat];
	if(!g_Inited || !decoder.name || !decoder.kernel || decoder.format == PC_TEX_FMT_NONE)
		return PC_TEX_FMT_NONE;

#ifdef DEBUG_OPENCL
	g_clsrc = clCreateBuffer(OpenCL::GetContext(), CL_MEM_READ_ONLY , 1024 * 1024 * sizeof(u32), NULL, NULL);
	g_cldst = clCreateBuffer(OpenCL::GetContext(), CL_MEM_WRITE_ONLY, 1024 * 1024 * sizeof(u32), NULL, NULL);
#endif

	clEnqueueWriteBuffer(OpenCL::GetCommandQueue(), g_clsrc, CL_TRUE, 0, (size_t)(width * height * decoder.sizeOfSrc), src, 0, NULL, NULL);

	clSetKernelArg(decoder.kernel, 0, sizeof(cl_mem), &g_cldst);
	clSetKernelArg(decoder.kernel, 1, sizeof(cl_mem), &g_clsrc);
	clSetKernelArg(decoder.kernel, 2, sizeof(cl_int), &width);

	size_t global[] = { (size_t)(width / decoder.xSkip), (size_t)(height / decoder.ySkip) };

	// No work-groups for now
	/*
	size_t local;
	err = clGetKernelWorkGroupInfo(kernelToRun, OpenCL::device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
	if(err)
		PanicAlert("Error obtaining work-group information");
	*/

	err = clEnqueueNDRangeKernel(OpenCL::GetCommandQueue(), decoder.kernel, 2, NULL, global, NULL, 0, NULL, NULL);
	if(err)
		OpenCL::HandleCLError(err, "Failed to enqueue kernel");

	clFinish(OpenCL::GetCommandQueue());

	clEnqueueReadBuffer(OpenCL::GetCommandQueue(), g_cldst, CL_TRUE, 0, (size_t)(width * height * decoder.sizeOfDst), dst, 0, NULL, NULL);

#ifdef DEBUG_OPENCL
	clReleaseMemObject(g_clsrc);
	clReleaseMemObject(g_cldst);
#endif

	return decoder.format;
}
