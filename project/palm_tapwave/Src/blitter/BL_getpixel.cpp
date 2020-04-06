/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_getpixel.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB555 GetPixel Functions --------------
#endif

COLORREF BL_GetPixel555(const void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcX, LONG srcY)
{
	DWORD dwNativeColor;
	COLORREF dwColor;
	
	// Get the pixel
	dwNativeColor = pxutil555::GetPixel((WORD*)src, srcXPitch, srcYPitch, srcX, srcY);
	
	// Convert native color to COLORREF
	dwColor = pxutil555::NativeToColorref(dwNativeColor);
	
	return dwColor;
}

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 GetPixel Functions --------------
#endif

COLORREF BL_GetPixel565(const void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcX, LONG srcY)
{
	DWORD dwNativeColor;
	COLORREF dwColor;
	
	// Get the pixel
	dwNativeColor = pxutil565::GetPixel((WORD*)src, srcXPitch, srcYPitch, srcX, srcY);
	
	// Convert native color to COLORREF
	dwColor = pxutil565::NativeToColorref(dwNativeColor);
	
	return dwColor;
}