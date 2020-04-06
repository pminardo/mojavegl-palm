/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_pixelutil.h"
#include "BL_fillrect.h"
#include "BL_bitblt.h"
#include "BL_tilebitblt.h"

/**************************************************************
* Helper Functions
**************************************************************/

namespace TileBitBlt565 {

	inline
	void BitBlt(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, LONG destX, LONG destY,
		void *src, LONG srcXPitch, LONG srcYPitch, LONG srcWidth, LONG srcHeight,
		RECT *pSrcRect, RECT *pClipRect, DWORD dwFlags, COLORREF dwColorKey,
		COLORREF dwFillColor)
	{
		RECT srcRect = *pSrcRect;
		LONG destRight, destBottom, srcRectWidth, srcRectHeight;
		
		/* Now handle negative values in the rectangles.
		    Handle the case where nothing is to be done.
		*/
		if ((destX   >= destWidth) ||
			(destY    >= destHeight) ||
			(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
			(srcRect.top >= srcHeight) ||
			(srcRect.left >= srcWidth))
		{
			return;
		}
		
		destRight = destX+srcRect.right-srcRect.left;
		destBottom = destY+srcRect.bottom-srcRect.top;
		
		// ------------------------------------------
		// Clipping
		// ------------------------------------------
		if (!IsRectEmpty(pClipRect))
		{
			// Handle clipping (left)
			if (destX < pClipRect->left)
			{
				srcRect.left += pClipRect->left-destX;
				destX = pClipRect->left;
			}
			
			// Handle clipping (right)
			if (destRight > pClipRect->right)
			{
				srcRect.right -= destRight - pClipRect->right;
			}
			
			// Handle clipping (top)
			if (destY < pClipRect->top)
			{
				srcRect.top += pClipRect->top-destY;
				destY = pClipRect->top;
			}
			
			// Handle clipping (bottom)
			if (destBottom > pClipRect->bottom)
			{
				srcRect.bottom -= destBottom - pClipRect->bottom;
			}
			
			// Make sure that there's something to blit
			if (IsRectEmpty(&srcRect))
				return;
		}

		srcRectWidth = srcRect.right - srcRect.left;
		srcRectHeight = srcRect.bottom - srcRect.top;
		
		// ------------------------------------------
		// Drawing
		// ------------------------------------------
		if (dwFlags & MGLBITBLT_KEYSRC)
		{
			DWORD dwNativeColorKey = pxutil565::ColorrefToNative(dwColorKey);
		
			if (dwFlags & MGLBITBLT_COLORFILL)
			{
				DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
			
				BL_BitBltKeySrcColorFill565(dest,
					src,
					destXPitch,
					destYPitch,
					srcXPitch,
					srcYPitch,
					destX,
					destY,
					destWidth,
					destHeight,
					srcRect.left,
					srcRect.top,
					srcRectWidth,
					srcRectHeight,
					dwNativeColorKey,
					dwNativeColorFill);
			}
			else
			{
				BL_BitBltKeySrc565(dest,
					src,
					destXPitch,
					destYPitch,
					srcXPitch,
					srcYPitch,
					destX,
					destY,
					destWidth,
					destHeight,
					srcRect.left,
					srcRect.top,
					srcRectWidth,
					srcRectHeight,
					dwNativeColorKey);
			}
		}
		else
		{
			if (dwFlags & MGLBITBLT_COLORFILL)
			{
				DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
				
				// This part is pretty easy - simple as FillRect().
				BL_FillRect565(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					destX,
					destY,
					srcRectWidth,
					srcRectHeight,
					dwNativeColorFill);
			}
			else
			{
				BL_BitBlt565(dest,
					src,
					destXPitch,
					destYPitch,
					srcXPitch,
					srcYPitch,
					destX,
					destY,
					destWidth,
					destHeight,
					srcRect.left,
					srcRect.top,
					srcRectWidth,
					srcRectHeight);
			}
		}
	}

}

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 TileBitBlt Functions --------------
#endif

void BL_TileBitBlt565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, LONG srcWidth, LONG srcHeight,
	RECT *pSrcRect, RECT *pClipRect, DWORD dwFlags,
	COLORREF dwColorKey, COLORREF dwFillColor)
{
	LONG x, y, right, bottom, srcRectWidth, srcRectHeight;
	RECT newClipRect;
	
	// Adjust the clipper to clip the drawing to pDestRect
	newClipRect.left = max(pDestRect->left, pClipRect->left);
	newClipRect.top = max(pDestRect->top, pClipRect->top);
	newClipRect.right = min(pDestRect->right, pClipRect->right);
	newClipRect.bottom = min(pDestRect->bottom, pClipRect->bottom);
	
	right = pDestRect->right;
	bottom = pDestRect->bottom;
	srcRectWidth = pSrcRect->right-pSrcRect->left;
	srcRectHeight = pSrcRect->bottom-pSrcRect->top;
	
	// BitBlt() each bitmap to form the tiled blit
	y = pDestRect->top;
	while (y < bottom)
	{
		x = pDestRect->left;
		while (x < right)
		{
			TileBitBlt565::BitBlt(dest, destXPitch, destYPitch,
				destWidth, destHeight, x, y, src, srcXPitch, srcYPitch, srcWidth, srcHeight,
				pSrcRect, &newClipRect, dwFlags, dwColorKey, dwFillColor);
			
			// Advance X
			x += srcRectWidth;
		}
		
		// Advance Y 
		y += srcRectHeight;
	}
}