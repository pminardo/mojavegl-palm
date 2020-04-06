/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_pixelutil.h"
#include "BL_stretchbltopacity.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 StretchBltOpacity Functions --------------
#endif

void BL_StretchBltOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwOpacity)
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
				*destp++ = pxutil565::AlphaBlend(*(srcp - (currXFixed >> 16)), *destp, dwOpacity);\
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
				*destp++ = pxutil565::AlphaBlend(*(srcp - (currXFixed >> 16)), *destp, dwOpacity);\
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
				*destp++ = pxutil565::AlphaBlend(*(srcp + (currXFixed >> 16)), *destp, dwOpacity);\
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
				*destp++ = pxutil565::AlphaBlend(*(srcp + (currXFixed >> 16)), *destp, dwOpacity);\
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

void BL_StretchBltOpacityKeySrc565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwNativeColorKey, DWORD dwOpacity)
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
				*destp = pxutil565::AlphaBlend(dwTempSrc16, *destp, dwOpacity);\
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
				*destp = pxutil565::AlphaBlend(dwTempSrc16, *destp, dwOpacity);\
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
				*destp = pxutil565::AlphaBlend(dwTempSrc16, *destp, dwOpacity);\
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
				*destp = pxutil565::AlphaBlend(dwTempSrc16, *destp, dwOpacity);\
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

void BL_StretchBltOpacityKeySrcColorFill565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwNativeColorKey, DWORD dwNativeColorFill,
	DWORD dwOpacity)
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
				*destp = pxutil565::AlphaBlend(dwNativeColorFill, *destp, dwOpacity);\
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
				*destp = pxutil565::AlphaBlend(dwNativeColorFill, *destp, dwOpacity);\
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
				*destp = pxutil565::AlphaBlend(dwNativeColorFill, *destp, dwOpacity);\
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
				*destp = pxutil565::AlphaBlend(dwNativeColorFill, *destp, dwOpacity);\
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