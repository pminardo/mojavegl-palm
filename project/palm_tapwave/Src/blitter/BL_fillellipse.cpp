/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_fillrect.h"
#include "BL_fillellipse.h"

/**************************************************************
* Helper Functions
**************************************************************/

namespace FillEllipse565 {

	inline
	void FillBounds(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, RECT *pClipper,
		LONG x, LONG y, LONG w, LONG h, DWORD dwNativeColor)
	{
		RECT rect;
		
		rect.left = x;
		rect.top = y;
		rect.right = x+w;
		rect.bottom = y+h;
		
		// Handle clipping for FillRect()
		if (!IsRectEmpty(pClipper))
		{
			// Clip the fill rectangle
			IntersectRect(&rect, &rect, pClipper);
			
			// If there's nothing to draw, then we're done.
			if (IsRectEmpty(&rect)) return;
		}
		
		BL_FillRect565(dest,
			destXPitch,
			destYPitch,
			destWidth,
			destHeight,
			rect.left,
			rect.top,
			rect.right-rect.left,
			rect.bottom-rect.top,
			dwNativeColor);
	}
	
	void FillBoundsOpacity(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, RECT *pClipper,
		LONG x, LONG y, LONG w, LONG h, DWORD dwNativeColor,
		DWORD dwOpacity)
	{
		RECT rect;
		
		rect.left = x;
		rect.top = y;
		rect.right = x+w;
		rect.bottom = y+h;
		
		// Handle clipping for FillRect()
		if (!IsRectEmpty(pClipper))
		{
			// Clip the fill rectangle
			IntersectRect(&rect, &rect, pClipper);
			
			// If there's nothing to draw, then we're done.
			if (IsRectEmpty(&rect)) return;
		}
		
		BL_FillRectOpacity565(dest,
			destXPitch,
			destYPitch,
			destWidth,
			destHeight,
			rect.left,
			rect.top,
			rect.right-rect.left,
			rect.bottom-rect.top,
			dwNativeColor,
			dwOpacity);
	}
}

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 FillEllipse Functions --------------
#endif

void BL_FillEllipse565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, RECT *pRect,
	COLORREF dwColor)
{
	DWORD dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	
	LONG left, top, width, height;
	
	left = pRect->left;
	top = pRect->top;
	width = (pRect->right-pRect->left)-1;
	height = (pRect->bottom-pRect->top)-1;
	
	LONG a = (width)>>1, b = (height)>>1,
		wod = (width&1)+1, hod = (height&1)+1,
		cx = left+a, cy = top+b,
		x = 0, y = b,
		ox = 0, oy = b,
		aa2 = (a*a)<<1, aa4 = aa2<<1, bb = (b*b)<<1,
		st = (aa2>>1)*(1-(b<<1)) + bb,
		tt = (bb>>1) - aa2*((b<<1)-1),
		pxl, dw, dh;
	
	if (width+1) while (y > 0)
	{
		if (st < 0)
		{
			st += bb*((x<<1)+3);
			tt += (bb<<1)*(++x);
		}
		else if (tt < 0)
		{
			st += bb*((x<<1)+3) - aa4*(y-1);
			pxl = cx-x;
			dw = (x<<1)+wod;
			tt += (bb<<1)*(++x) - aa2*(((y--)<<1)-3);
			dh = oy-y;
			
			FillEllipse565::FillBounds(dest, destXPitch, destYPitch,
				destWidth, destHeight, pClipper,
				pxl, cy-oy, dw, dh,
				dwNativeColor);

			FillEllipse565::FillBounds(dest, destXPitch, destYPitch,
				destWidth, destHeight, pClipper,
				pxl, cy+y+hod, dw, dh,
				dwNativeColor);
			
			ox = x;
			oy = y;
		}
		else
		{
			tt -= aa2*((y<<1)-3);
			st -= aa4*(--y);
		}
	}

	FillEllipse565::FillBounds(dest, destXPitch, destYPitch,
		destWidth, destHeight, pClipper,
		cx-a, cy-oy, width+1, (oy<<1)+hod,
		dwNativeColor);
}

void BL_FillEllipseOpacity565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, RECT *pRect,
	COLORREF dwColor, DWORD dwOpacity)
{
	DWORD dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	
	LONG left, top, width, height;
	
	left = pRect->left;
	top = pRect->top;
	width = (pRect->right-pRect->left)-1;
	height = (pRect->bottom-pRect->top)-1;
	
	LONG a = (width)>>1, b = (height)>>1,
		wod = (width&1)+1, hod = (height&1)+1,
		cx = left+a, cy = top+b,
		x = 0, y = b,
		ox = 0, oy = b,
		aa2 = (a*a)<<1, aa4 = aa2<<1, bb = (b*b)<<1,
		st = (aa2>>1)*(1-(b<<1)) + bb,
		tt = (bb>>1) - aa2*((b<<1)-1),
		pxl, dw, dh;
	
	if (width+1) while (y > 0)
	{
		if (st < 0)
		{
			st += bb*((x<<1)+3);
			tt += (bb<<1)*(++x);
		}
		else if (tt < 0)
		{
			st += bb*((x<<1)+3) - aa4*(y-1);
			pxl = cx-x;
			dw = (x<<1)+wod;
			tt += (bb<<1)*(++x) - aa2*(((y--)<<1)-3);
			dh = oy-y;
			
			FillEllipse565::FillBoundsOpacity(dest, destXPitch, destYPitch,
				destWidth, destHeight, pClipper,
				pxl, cy-oy, dw, dh,
				dwNativeColor, dwOpacity);

			FillEllipse565::FillBoundsOpacity(dest, destXPitch, destYPitch,
				destWidth, destHeight, pClipper,
				pxl, cy+y+hod, dw, dh,
				dwNativeColor, dwOpacity);
			
			ox = x;
			oy = y;
		}
		else
		{
			tt -= aa2*((y<<1)-3);
			st -= aa4*(--y);
		}
	}

	FillEllipse565::FillBoundsOpacity(dest, destXPitch, destYPitch,
		destWidth, destHeight, pClipper,
		cx-a, cy-oy, width+1, (oy<<1)+hod,
		dwNativeColor, dwOpacity);
}