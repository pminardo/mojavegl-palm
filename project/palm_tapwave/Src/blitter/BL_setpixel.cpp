/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_setpixel.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB555 SetPixel Functions --------------
#endif

void BL_SetPixel555(const void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, COLORREF dwColor)
{
	DWORD dwNativeColor;
			
	dwNativeColor = pxutil555::ColorrefToNative(dwColor);
	pxutil555::SetPixel((WORD*)dest, destXPitch, destYPitch, destX, destY, dwNativeColor);
}

void BL_SetPixelOpacity555(const void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, COLORREF dwColor, DWORD dwOpacity)
{
	DWORD dwNativeColor;
			
	dwNativeColor = pxutil555::ColorrefToNative(dwColor);
	pxutil555::SetPixelOpacity((WORD*)dest, destXPitch, destYPitch, destX, destY, dwNativeColor, dwOpacity);
}


#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 SetPixel Functions --------------
#endif

void BL_SetPixel565(const void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, COLORREF dwColor)
{
	DWORD dwNativeColor;
			
	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	pxutil565::SetPixel((WORD*)dest, destXPitch, destYPitch, destX, destY, dwNativeColor);
}

void BL_SetPixelOpacity565(const void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, COLORREF dwColor, DWORD dwOpacity)
{
	DWORD dwNativeColor;
			
	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	pxutil565::SetPixelOpacity((WORD*)dest, destXPitch, destYPitch, destX, destY, dwNativeColor, dwOpacity);
}