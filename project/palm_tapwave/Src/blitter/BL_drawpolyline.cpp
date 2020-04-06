/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_drawline.h"
#include "BL_drawpolyline.h"

/**************************************************************
* Helper Functions
**************************************************************/

namespace DrawPolyline565 {

	static
	void DrawLine(void *dest, LONG destXPitch, LONG destYPitch,
			LONG destWidth, LONG destHeight, LONG x1, LONG y1,
			LONG x2, LONG y2, RECT *pClipper, COLORREF dwColor, DWORD dwFlags)
	{
		POINT pt1, pt2;
	
		// -------------------------------
		// Clipping
		// -------------------------------
		
		pt1.x = x1;
		pt1.y = y1;
		pt2.x = x2;
		pt2.y = y2;
		
		// Clip the line coordinates
		if (!DrawLineHelper::ClipLine(pt1, pt2, *pClipper))
			return;
		
		// -------------------------------
		// Drawing
		// -------------------------------
		if (dwFlags & MGLDRAWPOLYLINE_ANTIALIAS)
		{
			BL_DrawWuLine565(dest, destXPitch, destYPitch, destWidth, destHeight,
				pt1.x, pt1.y, pt2.x, pt2.y, dwColor);
		}
		else
		{
			BL_DrawLine565(dest, destXPitch, destYPitch, destWidth, destHeight,
				pt1.x, pt1.y, pt2.x, pt2.y, dwColor);
		}
	}
}

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 DrawPolyline Functions --------------
#endif

void BL_DrawPolyline565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, COLORREF dwColor,
	POINT *pPointArray, DWORD dwNumPoints, DWORD dwFlags)
{
	LONG i;
	
	// Draw the polyline
	for (i = 0; i < dwNumPoints-1; i++)
	{
		DrawPolyline565::DrawLine(dest, destXPitch, destYPitch,
			destWidth, destHeight, pPointArray[i].x, pPointArray[i].y, pPointArray[i+1].x, pPointArray[i+1].y,
			pClipper, dwColor, dwFlags);
	}
}