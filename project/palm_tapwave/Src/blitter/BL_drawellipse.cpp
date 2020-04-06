/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_fillrect.h"
#include "BL_drawellipse.h"

/**************************************************************
* Helper Functions
**************************************************************/

namespace DrawEllipse565 {

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

	inline
	void FillBoundsQds(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, RECT *pClipper,
		LONG cx, LONG cy, LONG xl, LONG xr, LONG yt,
		LONG yb, LONG w, LONG h, DWORD dwNativeColor)
	{
		FillBounds(dest, destXPitch, destYPitch,
			destWidth, destHeight, pClipper,
			xr+cx, yt+cy, w, h,
			dwNativeColor);
			
		FillBounds(dest, destXPitch, destYPitch,
			destWidth, destHeight, pClipper,
			xr+cx, yb+cy, w, h,
			dwNativeColor);
			
		FillBounds(dest, destXPitch, destYPitch,
			destWidth, destHeight, pClipper,
			xl+cx, yb+cy, w, h,
			dwNativeColor);
			
		FillBounds(dest, destXPitch, destYPitch,
			destWidth, destHeight, pClipper,
			xl+cx, yt+cy, w, h,
			dwNativeColor);
	}
	
	static
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
	
	inline
	void FillBoundsQdsOpacity(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, RECT *pClipper,
		LONG cx, LONG cy, LONG xl, LONG xr, LONG yt,
		LONG yb, LONG w, LONG h, DWORD dwNativeColor,
		DWORD dwOpacity)
	{
		FillBoundsOpacity(dest, destXPitch, destYPitch,
			destWidth, destHeight, pClipper,
			xr+cx, yt+cy, w, h,
			dwNativeColor, dwOpacity);
			
		FillBoundsOpacity(dest, destXPitch, destYPitch,
			destWidth, destHeight, pClipper,
			xr+cx, yb+cy, w, h,
			dwNativeColor, dwOpacity);
			
		FillBoundsOpacity(dest, destXPitch, destYPitch,
			destWidth, destHeight, pClipper,
			xl+cx, yb+cy, w, h,
			dwNativeColor, dwOpacity);
			
		FillBoundsOpacity(dest, destXPitch, destYPitch,
			destWidth, destHeight, pClipper,
			xl+cx, yt+cy, w, h,
			dwNativeColor, dwOpacity);
	}
}

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 DrawEllipse Functions --------------
#endif

void BL_DrawEllipse565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, RECT *pRect,
	COLORREF dwColor)
{
	DWORD dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	
	// I did not write this code. Ported from:
	// http://www.walterzorn.com/jsgraphics/jsgraphics_e.htm
	
	LONG left, top, width, height;
	
	left = pRect->left;
	top = pRect->top;
	width = (pRect->right-pRect->left)-1;
	height = (pRect->bottom-pRect->top)-1;
	
	LONG a = width>>1, b = height>>1,
		wod = width&1, hod = (height&1)+1,
		cx = left+a, cy = top+b,
		x = 0, y = b,
		ox = 0, oy = b,
		aa = (a*a)<<1, bb = (b*b)<<1,
		st = (aa>>1)*(1-(b<<1)) + bb,
		tt = (bb>>1) - aa*((b<<1)-1),
		w, h;
	
	while (y > 0)
	{
		if (st < 0)
		{
			st += bb*((x<<1)+3);
			tt += (bb<<1)*(++x);
		}
		else if (tt < 0)
		{
			st += bb*((x<<1)+3) - (aa<<1)*(y-1);
			tt += (bb<<1)*(++x) - aa*(((y--)<<1)-3);
			w = x-ox;
			h = oy-y;
			
			if (w&2 && h&2)
			{
				DrawEllipse565::FillBoundsQds(dest, destXPitch, destYPitch,
					destWidth, destHeight, pClipper,
					cx, cy, -x+2, ox+wod, -oy, oy-1+hod, 1, 1,
					dwNativeColor);
					
				DrawEllipse565::FillBoundsQds(dest, destXPitch, destYPitch,
					destWidth, destHeight, pClipper,
					cx, cy, -x+1, x-1+wod, -y-1, y+hod, 1, 1,
					dwNativeColor);
			}
			else
			{
				DrawEllipse565::FillBoundsQds(dest, destXPitch, destYPitch,
					destWidth, destHeight, pClipper,
					cx, cy, -x+1, ox+wod, -oy, oy-h+hod, w, h,
					dwNativeColor);
			}
			
			ox = x;
			oy = y;
		}
		else
		{
			tt -= aa*((y<<1)-3);
			st -= (aa<<1)*(--y);
		}
	}
	
	DrawEllipse565::FillBounds(dest, destXPitch, destYPitch,
		destWidth, destHeight, pClipper,
		cx-a, cy-oy, a-ox+1, (oy<<1)+hod,
		dwNativeColor);
	
	DrawEllipse565::FillBounds(dest, destXPitch, destYPitch,
		destWidth, destHeight, pClipper,
		cx+ox+wod, cy-oy, a-ox+1, (oy<<1)+hod,
		dwNativeColor);
}

void BL_DrawEllipseOpacity565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, RECT *pRect,
	COLORREF dwColor, DWORD dwOpacity)
{
	DWORD dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	
	// I did not write this code... ported from:
	// http://www.walterzorn.com/jsgraphics/jsgraphics_e.htm
	
	LONG left, top, width, height;
	
	left = pRect->left;
	top = pRect->top;
	width = (pRect->right-pRect->left)-1;
	height = (pRect->bottom-pRect->top)-1;
	
	LONG a = width>>1, b = height>>1,
		wod = width&1, hod = (height&1)+1,
		cx = left+a, cy = top+b,
		x = 0, y = b,
		ox = 0, oy = b,
		aa = (a*a)<<1, bb = (b*b)<<1,
		st = (aa>>1)*(1-(b<<1)) + bb,
		tt = (bb>>1) - aa*((b<<1)-1),
		w, h;
	
	while (y > 0)
	{
		if (st < 0)
		{
			st += bb*((x<<1)+3);
			tt += (bb<<1)*(++x);
		}
		else if (tt < 0)
		{
			st += bb*((x<<1)+3) - (aa<<1)*(y-1);
			tt += (bb<<1)*(++x) - aa*(((y--)<<1)-3);
			w = x-ox;
			h = oy-y;
			
			if (w&2 && h&2)
			{
				DrawEllipse565::FillBoundsQdsOpacity(dest, destXPitch, destYPitch,
					destWidth, destHeight, pClipper,
					cx, cy, -x+2, ox+wod, -oy, oy-1+hod, 1, 1,
					dwNativeColor, dwOpacity);
					
				DrawEllipse565::FillBoundsQdsOpacity(dest, destXPitch, destYPitch,
					destWidth, destHeight, pClipper,
					cx, cy, -x+1, x-1+wod, -y-1, y+hod, 1, 1,
					dwNativeColor, dwOpacity);
			}
			else
			{
				DrawEllipse565::FillBoundsQdsOpacity(dest, destXPitch, destYPitch,
					destWidth, destHeight, pClipper,
					cx, cy, -x+1, ox+wod, -oy, oy-h+hod, w, h,
					dwNativeColor, dwOpacity);
			}
			
			ox = x;
			oy = y;
		}
		else
		{
			tt -= aa*((y<<1)-3);
			st -= (aa<<1)*(--y);
		}
	}
	
	DrawEllipse565::FillBoundsOpacity(dest, destXPitch, destYPitch,
		destWidth, destHeight, pClipper,
		cx-a, cy-oy, a-ox+1, (oy<<1)+hod,
		dwNativeColor, dwOpacity);
	
	DrawEllipse565::FillBoundsOpacity(dest, destXPitch, destYPitch,
		destWidth, destHeight, pClipper,
		cx+ox+wod, cy-oy, a-ox+1, (oy<<1)+hod,
		dwNativeColor, dwOpacity);
}