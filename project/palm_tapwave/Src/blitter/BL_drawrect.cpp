/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_drawline.h"
#include "BL_drawrect.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 DrawRect Functions --------------
#endif

void BL_DrawRect565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x1, LONG y1,
	LONG x2, LONG y2, COLORREF dwColor)
{
	BL_DrawHLine565(dest, destXPitch, destYPitch, destWidth, destHeight,
		pClipper, x1, y1, x2, dwColor);
		
	BL_DrawHLine565(dest, destXPitch, destYPitch, destWidth, destHeight,
		pClipper, x1, y2, x2, dwColor);
	
	BL_DrawVLine565(dest, destXPitch, destYPitch, destWidth, destHeight,
		pClipper, x1, y1+1, y2-1, dwColor);
		
	BL_DrawVLine565(dest, destXPitch, destYPitch, destWidth, destHeight,
		pClipper, x2, y1+1, y2-1, dwColor);
}

void BL_DrawRectOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x1, LONG y1,
	LONG x2, LONG y2, COLORREF dwColor, DWORD dwOpacity)
{
	BL_DrawHLineOpacity565(dest, destXPitch, destYPitch, destWidth, destHeight,
		pClipper, x1, y1, x2, dwColor, dwOpacity);
		
	BL_DrawHLineOpacity565(dest, destXPitch, destYPitch, destWidth, destHeight,
		pClipper, x1, y2, x2, dwColor, dwOpacity);
	
	BL_DrawVLineOpacity565(dest, destXPitch, destYPitch, destWidth, destHeight,
		pClipper, x1, y1+1, y2-1, dwColor, dwOpacity);
		
	BL_DrawVLineOpacity565(dest, destXPitch, destYPitch, destWidth, destHeight,
		pClipper, x2, y1+1, y2-1, dwColor, dwOpacity);
}