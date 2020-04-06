/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_pixelutil.h"
#include "BL_stretchalphabltsubtractive.h"

/**************************************************************
* Constants
**************************************************************/

// Defines the maximum visible opacity
#define MAXALPHA		248

/**************************************************************
* Helper Functions
**************************************************************/

namespace StretchAlphaBltSubtractive565 {

	inline
	DWORD BlendPixel(DWORD pixel, DWORD backpixel, DWORD alphapixel)
	{
		DWORD alpha = pxutil565::GetSurfaceAlphaValue(alphapixel);

		if(alpha)
		{
			if (alpha < MAXALPHA)
				return pxutil565::AlphaBlendSubtractive(pixel, backpixel, alpha);

			return pixel;
		}

		return backpixel;
	}

	inline
	DWORD BlendPixelOpacity(DWORD pixel, DWORD backpixel, DWORD alphapixel, DWORD opacity)
	{
		 DWORD alpha = pxutil565::GetSurfaceAlphaValue(alphapixel);
		        
	        // Average the alpha value
	        alpha = pxutil::AverageAlpha (alpha, opacity);
	    	
		if(alpha)
		{
			if (alpha < MAXALPHA)
				return pxutil565::AlphaBlendSubtractive(pixel, backpixel, alpha);

			return pixel;
		}
		
		return backpixel;
	}
}

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 StretchAlphaBltSubtractive Functions --------------
#endif

void BL_StretchAlphaBltSubtractive565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight, RECT *pDestRect,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect,
	LONG alphaXPitch, LONG alphaYPitch, RECT *pAlphaRect,
	RECT *pClipRect, DWORD dwFlags)
{
	LONG destX, destY, maxX, maxY;
	FLOAT fXScaleFactor, fYScaleFactor, fStartX, fCurrY;
	DWORD xScaleFactorFixed, yScaleFactorFixed, startXFixed, currYFixed; // all stored as 16.16 fixed point!
	LONG nScaledW, nSrcOffset;
	WORD *pDest16, *pDestEnd16, *pSrc16, *pAlpha16;
	
	destX = pDestRect->left;
	destY = pDestRect->top;
	
	// Max X and Y
	maxX = pDestRect->right;
	maxY = pDestRect->bottom;
	
	// Compute X scaling factor
	fXScaleFactor = (FLOAT)(pSrcRect->right - pSrcRect->left)/*srcRectWidth*/ /
		(FLOAT)(pDestRect->right - pDestRect->left)/*destRectWidth*/;
		
	// Compute Y scaling factor
	fYScaleFactor = (FLOAT)(pSrcRect->bottom - pSrcRect->top)/*srcRectHeight*/ /
		(FLOAT)(pDestRect->bottom - pDestRect->top)/*destRectHeight*/;

	fStartX = 0;	// source image starting X
	fCurrY = 0;	// source image current Y
	
	// -------------------------------------------
	// Clipping
	// -------------------------------------------
	if (!IsRectEmpty(pClipRect))
	{
		// Clip left portion
		if (destX < pClipRect->left)
		{
			fStartX = ((FLOAT)(pClipRect->left - destX))*fXScaleFactor;
			destX = pClipRect->left;
		}
		
		// Clip right portion
		if (maxX > pClipRect->right)
			maxX = pClipRect->right;

		// Clip top portion
		if (destY < pClipRect->top)
		{
			fCurrY = ((FLOAT)(pClipRect->top - destY))*fYScaleFactor;
			destY = 0;
		}

		// Clip bottom portion
		if (maxY > pClipRect->bottom)
			maxY = pClipRect->bottom;
	}
		
	// -------------------------------------------
	// 16.16 fixed point conversion
	// -------------------------------------------
	xScaleFactorFixed = (DWORD)(fXScaleFactor * (1 << 16));
	yScaleFactorFixed = (DWORD)(fYScaleFactor * (1 << 16));
	startXFixed = (DWORD)(fStartX * (1 << 16));
	currYFixed = (DWORD)(fCurrY * (1 << 16));

	// -------------------------------------------
	// Do the StretchAlphaBltSubtractive()!
	// -------------------------------------------
	
	// Compute scaled width
	nScaledW = maxX - destX;
	
	// Init src and destination pointers
	pDest16 = (WORD*)dest + destX + (destY*destYPitch);
	pDestEnd16 = pDest16 + ((maxY - destY)*destYPitch);
	
	// I tried to make an all-purpose blitter that would
	// deal with mirroring but it was painfully slow. For now,
	// we'll have separate blit loops for each of the mirroring operations. - ptm
	
	// Mirror LEFTRIGHT & UPDOWN
	if ((dwFlags & MGLSTRETCHALPHABLT_MIRRORLEFTRIGHT) &&
		(dwFlags & MGLSTRETCHALPHABLT_MIRRORUPDOWN))
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + ((pSrcRect->bottom-1) * srcYPitch);
		pAlpha16 = (WORD*)alpha + (pAlphaRect->right-1) + ((pAlphaRect->bottom-1) * alphaYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp, *alphap;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			alphap = pAlpha16 - ((currYFixed >> 16) * alphaYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				nSrcOffset = (currXFixed >> 16);\
				*destp++ = StretchAlphaBltSubtractive565::BlendPixel(*(srcp - nSrcOffset), *destp, *(alphap - nSrcOffset));\
				currXFixed += xScaleFactorFixed;\
			}
			
			BLITTER_DUFF_DEVICE_16x(nScaledW, BLIT_EXPR);
			
			#undef BLIT_EXPR

			// Advance Y position
			pDest16 += destYPitch;
			currYFixed += yScaleFactorFixed;
		}
	}
	// Mirror LEFTRIGHT
	else if (dwFlags & MGLSTRETCHALPHABLT_MIRRORLEFTRIGHT)
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + (pSrcRect->top * srcYPitch);
		pAlpha16 = (WORD*)alpha + (pAlphaRect->right-1) + (pAlphaRect->top * alphaYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp, *alphap;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 +  ((currYFixed >> 16) * srcYPitch);
			alphap = pAlpha16 +  ((currYFixed >> 16) * alphaYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				nSrcOffset = (currXFixed >> 16);\
				*destp++ = StretchAlphaBltSubtractive565::BlendPixel(*(srcp - nSrcOffset), *destp, *(alphap - nSrcOffset));\
				currXFixed += xScaleFactorFixed;\
			}
			
			BLITTER_DUFF_DEVICE_16x(nScaledW, BLIT_EXPR);
			
			#undef BLIT_EXPR

			// Advance Y position
			pDest16 += destYPitch;
			currYFixed += yScaleFactorFixed;
		}
	}
	// Mirror UPDOWN
	else if (dwFlags & MGLSTRETCHALPHABLT_MIRRORUPDOWN)
	{
		pSrc16 = (WORD*)src + pSrcRect->left + ((pSrcRect->bottom-1) * srcYPitch);
		
		pAlpha16 = (WORD*)alpha + pAlphaRect->left + ((pAlphaRect->bottom-1) * alphaYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp, *alphap;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			alphap = pAlpha16 - ((currYFixed >> 16) * alphaYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				nSrcOffset = (currXFixed >> 16);\
				*destp++ = StretchAlphaBltSubtractive565::BlendPixel(*(srcp + nSrcOffset), *destp, *(alphap + nSrcOffset));\
				currXFixed += xScaleFactorFixed;\
			}
			
			BLITTER_DUFF_DEVICE_16x(nScaledW, BLIT_EXPR);
			
			#undef BLIT_EXPR

			// Advance Y position
			pDest16 += destYPitch;
			currYFixed += yScaleFactorFixed;
		}
	}
	// No mirroring
	else
	{
		pSrc16 = (WORD*)src + pSrcRect->left + (pSrcRect->top * srcYPitch);
		pAlpha16 = (WORD*)alpha + pAlphaRect->left + (pAlphaRect->top * alphaYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp, *alphap;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 +  ((currYFixed >> 16) * srcYPitch);
			alphap = pAlpha16 +  ((currYFixed >> 16) * alphaYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				nSrcOffset = (currXFixed >> 16);\
				*destp++ = StretchAlphaBltSubtractive565::BlendPixel(*(srcp + nSrcOffset), *destp, *(alphap + nSrcOffset));\
				currXFixed += xScaleFactorFixed;\
			}
			
			BLITTER_DUFF_DEVICE_16x(nScaledW, BLIT_EXPR);
			
			#undef BLIT_EXPR

			// Advance Y position
			pDest16 += destYPitch;
			currYFixed += yScaleFactorFixed;
		}
	}
}

void BL_StretchAlphaBltSubtractiveOpacity565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight, RECT *pDestRect,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect,
	LONG alphaXPitch, LONG alphaYPitch, RECT *pAlphaRect,
	RECT *pClipRect, DWORD dwFlags, DWORD dwOpacity)
{
	LONG destX, destY, maxX, maxY;
	FLOAT fXScaleFactor, fYScaleFactor, fStartX, fCurrY;
	DWORD xScaleFactorFixed, yScaleFactorFixed, startXFixed, currYFixed; // all stored as 16.16 fixed point!
	LONG nScaledW, nSrcOffset;
	WORD *pDest16, *pDestEnd16, *pSrc16, *pAlpha16;
	
	destX = pDestRect->left;
	destY = pDestRect->top;
	
	// Max X and Y
	maxX = pDestRect->right;
	maxY = pDestRect->bottom;
	
	// Compute X scaling factor
	fXScaleFactor = (FLOAT)(pSrcRect->right - pSrcRect->left)/*srcRectWidth*/ /
		(FLOAT)(pDestRect->right - pDestRect->left)/*destRectWidth*/;
		
	// Compute Y scaling factor
	fYScaleFactor = (FLOAT)(pSrcRect->bottom - pSrcRect->top)/*srcRectHeight*/ /
		(FLOAT)(pDestRect->bottom - pDestRect->top)/*destRectHeight*/;

	fStartX = 0;	// source image starting X
	fCurrY = 0;	// source image current Y
	
	// -------------------------------------------
	// Clipping
	// -------------------------------------------
	if (!IsRectEmpty(pClipRect))
	{
		// Clip left portion
		if (destX < pClipRect->left)
		{
			fStartX = ((FLOAT)(pClipRect->left - destX))*fXScaleFactor;
			destX = pClipRect->left;
		}
		
		// Clip right portion
		if (maxX > pClipRect->right)
			maxX = pClipRect->right;

		// Clip top portion
		if (destY < pClipRect->top)
		{
			fCurrY = ((FLOAT)(pClipRect->top - destY))*fYScaleFactor;
			destY = 0;
		}

		// Clip bottom portion
		if (maxY > pClipRect->bottom)
			maxY = pClipRect->bottom;
	}
		
	// -------------------------------------------
	// 16.16 fixed point conversion
	// -------------------------------------------
	xScaleFactorFixed = (DWORD)(fXScaleFactor * (1 << 16));
	yScaleFactorFixed = (DWORD)(fYScaleFactor * (1 << 16));
	startXFixed = (DWORD)(fStartX * (1 << 16));
	currYFixed = (DWORD)(fCurrY * (1 << 16));

	// -------------------------------------------
	// Do the StretchAlphaBlt()!
	// -------------------------------------------
	
	// Compute scaled width
	nScaledW = maxX - destX;
	
	// Init src and destination pointers
	pDest16 = (WORD*)dest + destX + (destY*destYPitch);
	pDestEnd16 = pDest16 + ((maxY - destY)*destYPitch);
	
	// I tried to make an all-purpose blitter that would
	// deal with mirroring but it was painfully slow. For now,
	// we'll have separate blit loops for each of the mirroring operations. - ptm
	
	// Mirror LEFTRIGHT & UPDOWN
	if ((dwFlags & MGLSTRETCHALPHABLT_MIRRORLEFTRIGHT) &&
		(dwFlags & MGLSTRETCHALPHABLT_MIRRORUPDOWN))
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + ((pSrcRect->bottom-1) * srcYPitch);
		pAlpha16 = (WORD*)alpha + (pAlphaRect->right-1) + ((pAlphaRect->bottom-1) * alphaYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp, *alphap;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			alphap = pAlpha16 - ((currYFixed >> 16) * alphaYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				nSrcOffset = (currXFixed >> 16);\
				*destp++ = StretchAlphaBltSubtractive565::BlendPixelOpacity(*(srcp - nSrcOffset), *destp, *(alphap - nSrcOffset), dwOpacity);\
				currXFixed += xScaleFactorFixed;\
			}
			
			BLITTER_DUFF_DEVICE_16x(nScaledW, BLIT_EXPR);
			
			#undef BLIT_EXPR

			// Advance Y position
			pDest16 += destYPitch;
			currYFixed += yScaleFactorFixed;
		}
	}
	// Mirror LEFTRIGHT
	else if (dwFlags & MGLSTRETCHALPHABLT_MIRRORLEFTRIGHT)
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + (pSrcRect->top * srcYPitch);
		pAlpha16 = (WORD*)alpha + (pAlphaRect->right-1) + (pAlphaRect->top * alphaYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp, *alphap;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 +  ((currYFixed >> 16) * srcYPitch);
			alphap = pAlpha16 +  ((currYFixed >> 16) * alphaYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				nSrcOffset = (currXFixed >> 16);\
				*destp++ = StretchAlphaBltSubtractive565::BlendPixelOpacity(*(srcp - nSrcOffset), *destp, *(alphap - nSrcOffset), dwOpacity);\
				currXFixed += xScaleFactorFixed;\
			}
			
			BLITTER_DUFF_DEVICE_16x(nScaledW, BLIT_EXPR);
			
			#undef BLIT_EXPR

			// Advance Y position
			pDest16 += destYPitch;
			currYFixed += yScaleFactorFixed;
		}
	}
	// Mirror UPDOWN
	else if (dwFlags & MGLSTRETCHALPHABLT_MIRRORUPDOWN)
	{
		pSrc16 = (WORD*)src + pSrcRect->left + ((pSrcRect->bottom-1) * srcYPitch);
		
		pAlpha16 = (WORD*)alpha + pAlphaRect->left + ((pAlphaRect->bottom-1) * alphaYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp, *alphap;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			alphap = pAlpha16 - ((currYFixed >> 16) * alphaYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				nSrcOffset = (currXFixed >> 16);\
				*destp++ = StretchAlphaBltSubtractive565::BlendPixelOpacity(*(srcp + nSrcOffset), *destp, *(alphap + nSrcOffset), dwOpacity);\
				currXFixed += xScaleFactorFixed;\
			}
			
			BLITTER_DUFF_DEVICE_16x(nScaledW, BLIT_EXPR);
			
			#undef BLIT_EXPR

			// Advance Y position
			pDest16 += destYPitch;
			currYFixed += yScaleFactorFixed;
		}
	}
	// No mirroring
	else
	{
		pSrc16 = (WORD*)src + pSrcRect->left + (pSrcRect->top * srcYPitch);
		pAlpha16 = (WORD*)alpha + pAlphaRect->left + (pAlphaRect->top * alphaYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp, *alphap;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 +  ((currYFixed >> 16) * srcYPitch);
			alphap = pAlpha16 +  ((currYFixed >> 16) * alphaYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				nSrcOffset = (currXFixed >> 16);\
				*destp++ = StretchAlphaBltSubtractive565::BlendPixelOpacity(*(srcp + nSrcOffset), *destp, *(alphap + nSrcOffset), dwOpacity);\
				currXFixed += xScaleFactorFixed;\
			}
			
			BLITTER_DUFF_DEVICE_16x(nScaledW, BLIT_EXPR);
			
			#undef BLIT_EXPR

			// Advance Y position
			pDest16 += destYPitch;
			currYFixed += yScaleFactorFixed;
		}
	}
}