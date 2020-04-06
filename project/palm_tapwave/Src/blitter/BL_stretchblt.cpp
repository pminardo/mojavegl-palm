/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_pixelutil.h"
#include "BL_stretchblt.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ Generic 16-bit StretchBlt Functions --------------
#endif

void _BL_StretchBlt16(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags)
{
	LONG destX, destY, maxX, maxY;
	FLOAT fXScaleFactor, fYScaleFactor, fStartX, fCurrY;
	DWORD xScaleFactorFixed, yScaleFactorFixed, startXFixed, currYFixed; // all stored as 16.16 fixed point!
	LONG nScaledW;
	WORD *pDest16, *pDestEnd16, *pSrc16;
	
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
	// Do the StretchBlt()!
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
	if ((dwFlags & MGLSTRETCHBLT_MIRRORLEFTRIGHT) &&
		(dwFlags & MGLSTRETCHBLT_MIRRORUPDOWN))
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + ((pSrcRect->bottom-1) * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				*destp++ = *(srcp - (currXFixed >> 16));\
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
	else if (dwFlags & MGLSTRETCHBLT_MIRRORLEFTRIGHT)
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + (pSrcRect->top * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 +  ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				*destp++ = *(srcp - (currXFixed >> 16));\
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
	else if (dwFlags & MGLSTRETCHBLT_MIRRORUPDOWN)
	{
		pSrc16 = (WORD*)src + pSrcRect->left + ((pSrcRect->bottom-1) * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				*destp++ = *(srcp + (currXFixed >> 16));\
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
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 + ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				*destp++ = *(srcp + (currXFixed >> 16));\
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

void _BL_StretchBltKeySrc16(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwNativeColorKey)
{
	LONG destX, destY, maxX, maxY;
	FLOAT fXScaleFactor, fYScaleFactor, fStartX, fCurrY;
	DWORD xScaleFactorFixed, yScaleFactorFixed, startXFixed, currYFixed; // all stored as 16.16 fixed point!
	LONG nScaledW;
	WORD *pDest16, *pDestEnd16, *pSrc16;
	DWORD dwTempSrc16;
	
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
	// Do the StretchBlt()!
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
	if ((dwFlags & MGLSTRETCHBLT_MIRRORLEFTRIGHT) &&
		(dwFlags & MGLSTRETCHBLT_MIRRORUPDOWN))
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + ((pSrcRect->bottom-1) * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				if ((dwTempSrc16 = *(srcp - (currXFixed >> 16))) != dwNativeColorKey)\
				*destp = dwTempSrc16;\
			\
				++destp;\
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
	else if (dwFlags & MGLSTRETCHBLT_MIRRORLEFTRIGHT)
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + (pSrcRect->top * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 +  ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				if ((dwTempSrc16 = *(srcp - (currXFixed >> 16))) != dwNativeColorKey)\
				*destp = dwTempSrc16;\
			\
				++destp;\
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
	else if (dwFlags & MGLSTRETCHBLT_MIRRORUPDOWN)
	{
		pSrc16 = (WORD*)src + pSrcRect->left + ((pSrcRect->bottom-1) * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				if ((dwTempSrc16 = *(srcp + (currXFixed >> 16))) != dwNativeColorKey)\
				*destp = dwTempSrc16;\
			\
				++destp;\
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
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 + ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				if ((dwTempSrc16 = *(srcp + (currXFixed >> 16))) != dwNativeColorKey)\
				*destp = dwTempSrc16;\
			\
				++destp;\
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

void _BL_StretchBltKeySrcColorFill16(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwNativeColorKey, DWORD dwNativeColorFill)
{
	LONG destX, destY, maxX, maxY;
	FLOAT fXScaleFactor, fYScaleFactor, fStartX, fCurrY;
	DWORD xScaleFactorFixed, yScaleFactorFixed, startXFixed, currYFixed; // all stored as 16.16 fixed point!
	LONG nScaledW;
	WORD *pDest16, *pDestEnd16, *pSrc16;
	
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
	// Do the StretchBlt()!
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
	if ((dwFlags & MGLSTRETCHBLT_MIRRORLEFTRIGHT) &&
		(dwFlags & MGLSTRETCHBLT_MIRRORUPDOWN))
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + ((pSrcRect->bottom-1) * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				if (*(srcp - (currXFixed >> 16)) != dwNativeColorKey)\
				*destp = dwNativeColorFill;\
			\
				++destp;\
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
	else if (dwFlags & MGLSTRETCHBLT_MIRRORLEFTRIGHT)
	{
		pSrc16 = (WORD*)src + (pSrcRect->right-1) + (pSrcRect->top * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 +  ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				if (*(srcp - (currXFixed >> 16)) != dwNativeColorKey)\
				*destp = dwNativeColorFill;\
			\
				++destp;\
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
	else if (dwFlags & MGLSTRETCHBLT_MIRRORUPDOWN)
	{
		pSrc16 = (WORD*)src + pSrcRect->left + ((pSrcRect->bottom-1) * srcYPitch);
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 - ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				if (*(srcp + (currXFixed >> 16)) != dwNativeColorKey)\
				*destp = dwNativeColorFill;\
			\
				++destp;\
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
	
		while (pDest16 < pDestEnd16)
		{
			WORD *destp, *srcp;
			DWORD currXFixed;
			
			// Init pointers
			destp = pDest16;
			srcp = pSrc16 + ((currYFixed >> 16) * srcYPitch);
			
			currXFixed = startXFixed;
			
			// -------------------------------------------
			// Process each pixel in the row
			// -------------------------------------------
			#define BLIT_EXPR\
			{\
				if (*(srcp + (currXFixed >> 16)) != dwNativeColorKey)\
				*destp = dwNativeColorFill;\
			\
				++destp;\
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

/*void BL_StretchBltTransform565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, const RECT *pDestRect, DWORD *tmatrix, void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcRectWidth, LONG srcRectHeight)
{
 
    DWORD y1 = (pDestRect->top << 6) + (1<<5);
    DWORD y2 = (pDestRect->bottom << 6) + (1<<5);
    DWORD x1 = (pDestRect->left << 6) + (1<<5);
    DWORD x2 = (pDestRect->right << 6) + (1<<5);

    DWORD u0 = x1 * tmatrix[0] + y1 * tmatrix[2] + tmatrix[4];
    DWORD v0 = x1 * tmatrix[1] + y1 * tmatrix[3] + tmatrix[5];

    DWORD dudx = tmatrix[0] << 6;
    DWORD dvdx = tmatrix[1] << 6;
    DWORD dudy = tmatrix[2] << 6;
    DWORD dvdy = tmatrix[3] << 6;

    for (DWORD dy = y1; dy != y2; dy += (1<<6) )
    {
        WORD *pDest = pxutil565::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
        
        DWORD u = u0;
        DWORD v = v0;

        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
        {
		DWORD ix = u >> 16;	// src x
		DWORD iy = v >> 16;	// src y
		
		// Perform the pixel copy
		WORD *pSrc = pxutil565::GetBuffPixels(src, srcXPitch, srcYPitch, ix, iy);
		*pDest = *pSrc;

		u += dudx;
		v += dvdx;
		pDest += destXPitch;
        }

        u0 += dudy;
        v0 += dvdy;
    }
}*/