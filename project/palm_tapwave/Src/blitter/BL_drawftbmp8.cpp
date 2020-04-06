/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_drawftbmp8.h"

/**************************************************************
* Constants
**************************************************************/

// Defines the maximum visible opacity
#define MAXALPHA		248

/**************************************************************
* Helper Functions
**************************************************************/

namespace DrawFTBmp8_565 {

	inline
	DWORD BlendPixel(DWORD pixel, DWORD backpixel, DWORD alpha)
	{
		if(alpha)
		{
			if (alpha < MAXALPHA)
				return pxutil565::AlphaBlend(pixel, backpixel, alpha);

			return pixel;
		}

		return backpixel;
	}

	inline
	DWORD BlendPixelOpacity(DWORD pixel, DWORD backpixel, DWORD alpha, DWORD opacity)
	{
	        // Average the alpha value
	        alpha = pxutil::AverageAlpha (alpha, opacity);
	    	
		if(alpha)
		{
			if (alpha < MAXALPHA)
				return pxutil565::AlphaBlend(pixel, backpixel, alpha);

			return pixel;
		}
		
		return backpixel;
	}
}

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 DrawFTBmp8 Functions --------------
#endif

void BL_DrawFTBmp8_565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcPitch, LONG destX, LONG destY, LONG destWidth, LONG destHeight,
	LONG srcX, LONG srcY, LONG srcWidth, LONG srcHeight, DWORD dwNativeColor)
{
	WORD *pDest16 = (WORD*)dest;
	BYTE *pSrc8 = (BYTE*)src;
	DWORD dwDestOffset;
	DWORD dwSrcOffset;
	
	dwDestOffset = destYPitch - srcWidth;
	dwSrcOffset = srcPitch - srcWidth;
	
	// Init dest and src pointers
	pDest16 += destX + (destY * destYPitch);
	pSrc8 += srcX + (srcY * srcPitch);

	while (srcHeight != 0) {
	
		#define BLIT_EXPR\
		{\
			*pDest16 = DrawFTBmp8_565::BlendPixel(dwNativeColor, *pDest16, *pSrc8++);\
			pDest16 += destXPitch;\
		}
		
		BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
		
		#undef BLIT_EXPR
	
		pDest16 += dwDestOffset;
		pSrc8 += dwSrcOffset;
		--srcHeight;
	}
}

void BL_DrawFTBmp8Opacity_565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcPitch, LONG destX, LONG destY, LONG destWidth, LONG destHeight,
	LONG srcX, LONG srcY, LONG srcWidth, LONG srcHeight, DWORD dwNativeColor, DWORD dwOpacity)
{
	WORD *pDest16 = (WORD*)dest;
	BYTE *pSrc8 = (BYTE*)src;
	DWORD dwDestOffset;
	DWORD dwSrcOffset;
	
	dwDestOffset = destYPitch - srcWidth;
	dwSrcOffset = srcPitch - srcWidth;
	
	// Init dest and src pointers
	pDest16 += destX + (destY * destYPitch);
	pSrc8 += srcX + (srcY * srcPitch);

	while (srcHeight != 0) {
	
		#define BLIT_EXPR\
		{\
			*pDest16 = DrawFTBmp8_565::BlendPixelOpacity(dwNativeColor, *pDest16, *pSrc8++, dwOpacity);\
			pDest16 += destXPitch;\
		}
		
		BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
		
		#undef BLIT_EXPR
	
		pDest16 += dwDestOffset;
		pSrc8 += dwSrcOffset;
		--srcHeight;
	}
}