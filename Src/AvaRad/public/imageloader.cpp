//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================

#ifdef _WIN32
#include <windows.h>
#endif
#include "imageloader.h"
#include "basetypes.h"
#include "tier0/dbg.h"
#include <malloc.h>
#include <memory.h>
//#include "s3_intrf.h"
#include "mathlib.h"
#include "vector.h"
#include "utlmemory.h"
#include "tier0/memdbgon.h"

// Define this in your project settings if you want higher-quality/slower downsampling.
//#ifdef IMAGELOADER_NICE_FILTER


//-----------------------------------------------------------------------------
// Various important function types for each color format
//-----------------------------------------------------------------------------

typedef void (*UserFormatToRGBA8888Func_t )( unsigned char *src, unsigned char *dst, int numPixels );
typedef void (*RGBA8888ToUserFormatFunc_t )( unsigned char *src, unsigned char *dst, int numPixels );
typedef void (*GenMipMapLevelFunc_t )( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );

static ImageFormatInfo_t g_ImageFormatInfo[] =
{
	{ "IMAGE_FORMAT_RGBA8888",	4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_RGBA8888,
	{ "IMAGE_FORMAT_ABGR8888",	4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_ABGR8888, 
	{ "IMAGE_FORMAT_RGB888",	3, 8, 8, 8, 0, false }, // IMAGE_FORMAT_RGB888,
	{ "IMAGE_FORMAT_BGR888",	3, 8, 8, 8, 0, false }, // IMAGE_FORMAT_BGR888,
	{ "IMAGE_FORMAT_RGB565",	2, 5, 6, 5, 0, false }, // IMAGE_FORMAT_RGB565, 
	{ "IMAGE_FORMAT_I8",		1, 0, 0, 0, 0, false }, // IMAGE_FORMAT_I8,
	{ "IMAGE_FORMAT_IA88",		2, 0, 0, 0, 8, false }, // IMAGE_FORMAT_IA88
	{ "IMAGE_FORMAT_P8",		1, 0, 0, 0, 0, false }, // IMAGE_FORMAT_P8
	{ "IMAGE_FORMAT_A8",		1, 0, 0, 0, 8, false }, // IMAGE_FORMAT_A8
	{ "IMAGE_FORMAT_RGB888_BLUESCREEN", 3, 8, 8, 8, 0, false },	// IMAGE_FORMAT_RGB888_BLUESCREEN
	{ "IMAGE_FORMAT_BGR888_BLUESCREEN", 3, 8, 8, 8, 0, false },	// IMAGE_FORMAT_BGR888_BLUESCREEN
	{ "IMAGE_FORMAT_ARGB8888",	4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_ARGB8888
	{ "IMAGE_FORMAT_BGRA8888",	4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_BGRA8888
	{ "IMAGE_FORMAT_DXT1",		0, 0, 0, 0, 0, true }, // IMAGE_FORMAT_DXT1
	{ "IMAGE_FORMAT_DXT3",		0, 0, 0, 0, 8, true }, // IMAGE_FORMAT_DXT3
	{ "IMAGE_FORMAT_DXT5",		0, 0, 0, 0, 8, true }, // IMAGE_FORMAT_DXT5
	{ "IMAGE_FORMAT_BGRX8888",	4, 8, 8, 8, 0, false }, // IMAGE_FORMAT_BGRX8888
	{ "IMAGE_FORMAT_BGR565",	2, 5, 6, 5, 0, false }, // IMAGE_FORMAT_BGR565
	{ "IMAGE_FORMAT_BGRX5551",	2, 5, 5, 5, 0, false }, // IMAGE_FORMAT_BGRX5551
	{ "IMAGE_FORMAT_BGRA4444",	2, 4, 4, 4, 4, false },	 // IMAGE_FORMAT_BGRA4444
	{ "IMAGE_FORMAT_DXT1_ONEBITALPHA",		0, 0, 0, 0, 0, true }, // IMAGE_FORMAT_DXT1_ONEBITALPHA
	{ "IMAGE_FORMAT_BGRA5551",	2, 5, 5, 5, 1, false }, // IMAGE_FORMAT_BGRA5551
	{ "IMAGE_FORMAT_UV88",	    2, 8, 8, 0, 0, false }, // IMAGE_FORMAT_UV88
	{ "IMAGE_FORMAT_UVWQ8888",	    4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_UV88
	{ "IMAGE_FORMAT_RGBA16161616F",	    8, 16, 16, 16, 16, false }, // IMAGE_FORMAT_UV88
};


