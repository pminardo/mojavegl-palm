/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_drawline.h"
#include "BL_fillpolygon.h"

/**************************************************************
* Helper Functions
**************************************************************/

namespace FillPolygon565 {

	inline
	void DrawLine(void *dest, LONG destXPitch, LONG destYPitch,
			LONG destWidth, LONG destHeight, LONG x1, LONG y1,
			LONG x2, LONG y2, RECT *pClipper, COLORREF dwColor, DWORD dwFlags,
			DWORD dwOpacity)
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
		if (dwFlags & MGLFILLPOLYGON_OPACITY)
		{
			BL_DrawLineOpacity565(dest, destXPitch, destYPitch, destWidth, destHeight,
				pt1.x, pt1.y, pt2.x, pt2.y, dwColor, dwOpacity);
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
#pragma mark ------------ RGB565 FillPolygon Functions --------------
#endif

void BL_FillPolygon565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, COLORREF dwColor,
	POINT *pPointArray, DWORD dwNumPoints, DWORD dwFlags, DWORD dwOpacity)
{
	LONG i, index, j;
	LONG y;
	LONG miny, maxy;
	LONG x1, y1, x2, y2;
	LONG ind1, ind2;
	LONG ints;
	LONG *polyInts;
	
	miny = pPointArray[0].y;
	maxy = pPointArray[0].y;
	
	for (i = 1; i < dwNumPoints; i++)
	{
		if (pPointArray[i].y < miny)
			miny = pPointArray[i].y;

		if (pPointArray[i].y > maxy)
			maxy = pPointArray[i].y;
	}
	
	// Optimize for clipping
	miny = max(pClipper->top, miny);
	maxy = min(pClipper->bottom-1, maxy);
	
	polyInts = (LONG*)mgl_malloc(sizeof(LONG)*dwNumPoints);
	
	// This should *NEVER* fail, but it's best to handle this just in case... - ptm
	if (!polyInts) return;	// done drawing!
	
	for (y = miny; y <= maxy; y++)
	{
		ints = 0;
		for (i = 0; i < dwNumPoints; i++)
		{
			if (!i)
			{
				ind1 = dwNumPoints-1;
				ind2 = 0;
			}
			else
			{
				ind1 = i-1;
				ind2 = i;
			}
			
			y1 = pPointArray[ind1].y;
			y2 = pPointArray[ind2].y;

			if (y1 < y2)
			{
				x1 = pPointArray[ind1].x;
				x2 = pPointArray[ind2].x;
			}
			else if (y1 > y2)
			{
				y2 = pPointArray[ind1].y;
				y1 = pPointArray[ind2].y;
				x2 = pPointArray[ind1].x;
				x1 = pPointArray[ind2].x;
			}
			else
				continue;

			  if (((y >= y1) && (y < y2)) ||
			  	((y == maxy) && (y > y1) && (y <= y2)))
				{
				LONG yr1 = y - y1;
				LONG xr1 = x2 - x1;
				LONG yxr = (yr1) * (xr1);
				LONG yr2 = y2 - y1;
				
				  polyInts[ints++] = (LONG) ((DOUBLE) (yxr) /
					(DOUBLE) (yr2) + 0.5 + x1);
				}
			  /*else if ((y == maxy) && (y > y1) && (y <= y2))
				{
				LONG yr1 = y - y1;
				LONG xr1 = x2 - x1;
				LONG yxr = (yr1) * (xr1);
				LONG yr2 = y2 - y1;
				
				  polyInts[ints++] = (LONG) ((DOUBLE) (yxr) /
					(DOUBLE) (yr2) + 0.5 + x1);
				}*/
		}
		  
		//	2.0.26: polygons pretty much always have less than 100 points,
		//	and most of the time they have considerably less. For such trivial
		//	cases, insertion sort is a good choice. Also a good choice for
		//	future implementations that may wish to indirect through a table.
		  
		  for (i = 1; (i < ints); i++) {
			index = polyInts[i];
			j = i;
			while ((j > 0) && (polyInts[j - 1] > index)) {
			  polyInts[j] = polyInts[j - 1];
			  j--;
			}
			polyInts[j] = index;
		  }

		for (i = 0; i < ints; i+=2)
		{
			FillPolygon565::DrawLine(dest, destXPitch, destYPitch, destWidth, destHeight,
				polyInts[i], y, polyInts[i + 1], y, pClipper, dwColor, dwFlags, dwOpacity);
		}
	}
	
	mgl_free(polyInts);
}