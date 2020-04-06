/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_drawline.h"
#include "BL_drawbezier.h"

/**************************************************************
* Constants
**************************************************************/

#define BZS	120 //Bezier Scaler for fixed point math

/**************************************************************
* Helper Functions
**************************************************************/

namespace DrawBezier565 {

	inline
	void DrawSegment(void *dest, LONG destXPitch, LONG destYPitch,
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
		if (dwFlags & MGLDRAWBEZIER_ANTIALIAS)
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
#pragma mark ------------ RGB565 DrawBezier Functions --------------
#endif

void BL_DrawBezier565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, COLORREF dwColor,
	POINT *pPoints /*ptr to 4 element POINT array*/, DWORD dwFlags)
{
	// This code has been optimized and may be difficult to read
	const LONG k = BZS/30;
	const LONG bzsSquared = (BZS*BZS);
	const LONG bzsCubed = (BZS*BZS*BZS);
	const LONG bzsMul3 = (3*BZS);
	const LONG bzsMulNegative6 = (-6*BZS);
	
	LONG pt0xMulBzsCubed = pPoints[0].x*(bzsCubed);
	LONG pt0yMulBzsCubed = pPoints[0].y*(bzsCubed);
	
	LONG x1, x2, y1, y2, t;
	
	x1 = pPoints[0].x;
	y1 = pPoints[0].y;
	
	for (t = k; t<= BZS+k; t += k)
	{
		// Use Berstein polynomials
		x2 = (pt0xMulBzsCubed +
			t*(pPoints[0].x*(-3*bzsSquared)+t*((bzsMul3)*pPoints[0].x-pPoints[0].x*t)))+
			t*(pPoints[1].x*(3*bzsSquared)+ t*((bzsMulNegative6)*pPoints[1].x+pPoints[1].x*3*t))+
			t*t*(pPoints[2].x*(bzsMul3)-pPoints[2].x*3*t)+
			t*t*t*pPoints[3].x;
		
		x2 /= (bzsCubed);
		
		y2 = (pt0yMulBzsCubed +
			t*(pPoints[0].y*(-3*bzsSquared)+t*((bzsMul3)*pPoints[0].y-pPoints[0].y*t)))+
			t*(pPoints[1].y*(3*bzsSquared)+t*((bzsMulNegative6)*pPoints[1].y+ pPoints[1].y*3*t))+
			t*t*(pPoints[2].y*(bzsMul3)-pPoints[2].y*3*t)+
			t*t*t*pPoints[3].y;
		
		y2 /= (bzsCubed);
		
		// Draw curve segment
		DrawBezier565::DrawSegment(dest, destXPitch, destYPitch, destWidth, destHeight,
			x1, y1, x2, y2, pClipper, dwColor, dwFlags);
		
		// Advance coordinates
		x1 = x2;
		y1 = y2;
	}
}