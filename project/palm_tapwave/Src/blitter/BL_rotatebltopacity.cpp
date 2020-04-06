/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_rotatebltopacity.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ Generic 16-bit RotateBlt Functions --------------
#endif

void BL_RotateBltOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcRectWidth, LONG srcRectHeight, DWORD dwOpacity)
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
    
    if (dwOpacity == OPACITY_50)
    {
    	    for (DWORD dy = y1; dy != y2; dy += (1<<6) )
	    {
	        WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
	        
	        DWORD u = u0;
	        DWORD v = v0;

	        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
	        {
			DWORD ix = u >> 16;	// src x
			DWORD iy = v >> 16;	// src y
			
			// Perform the pixel copy
			if (ix < srcRectWidth && iy < srcRectHeight)
			{
				WORD *pSrc = pxutil16bit::GetBuffPixels(src, srcXPitch, srcYPitch, ix, iy);
				*pDest = pxutil565::AlphaBlendFast(*pSrc, *pDest);
			}

			u += dudx;
			v += dvdx;
			pDest += destXPitch;
	        }

	        u0 += dudy;
	        v0 += dvdy;
	    }
    }
    else
    {
	    for (DWORD dy = y1; dy != y2; dy += (1<<6) )
	    {
	        WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
	        
	        DWORD u = u0;
	        DWORD v = v0;

	        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
	        {
			DWORD ix = u >> 16;	// src x
			DWORD iy = v >> 16;	// src y
			
			// Perform the pixel copy
			if (ix < srcRectWidth && iy < srcRectHeight)
			{
				WORD *pSrc = pxutil16bit::GetBuffPixels(src, srcXPitch, srcYPitch, ix, iy);
				*pDest = pxutil565::AlphaBlend(*pSrc, *pDest, dwOpacity);
			}

			u += dudx;
			v += dvdx;
			pDest += destXPitch;
	        }

	        u0 += dudy;
	        v0 += dvdy;
	    }
   }
}

void BL_RotateBltOpacityKeySrc565(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcRectWidth, LONG srcRectHeight, DWORD dwNativeColorKey, DWORD dwOpacity)
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
    
    DWORD dwTempSrc16;
    
    if (dwOpacity == OPACITY_50)
    {
            for (DWORD dy = y1; dy != y2; dy += (1<<6) )
	    {
	        WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
	        
	        DWORD u = u0;
	        DWORD v = v0;

	        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
	        {
			DWORD ix = u >> 16;	// src x
			DWORD iy = v >> 16;	// src y
			
			// Perform the pixel copy
			if (ix < srcRectWidth && iy < srcRectHeight)
			{
				WORD *pSrc = pxutil16bit::GetBuffPixels(src, srcXPitch, srcYPitch, ix, iy);
				
				if ((dwTempSrc16 = *pSrc) != dwNativeColorKey)
					*pDest = pxutil565::AlphaBlendFast(dwTempSrc16, *pDest);
			}

			u += dudx;
			v += dvdx;
			pDest += destXPitch;
	        }

	        u0 += dudy;
	        v0 += dvdy;
	    }
    }
    else
    {
	    for (DWORD dy = y1; dy != y2; dy += (1<<6) )
	    {
	        WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
	        
	        DWORD u = u0;
	        DWORD v = v0;

	        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
	        {
			DWORD ix = u >> 16;	// src x
			DWORD iy = v >> 16;	// src y
			
			// Perform the pixel copy
			if (ix < srcRectWidth && iy < srcRectHeight)
			{
				WORD *pSrc = pxutil16bit::GetBuffPixels(src, srcXPitch, srcYPitch, ix, iy);
				
				if ((dwTempSrc16 = *pSrc) != dwNativeColorKey)
					*pDest = pxutil565::AlphaBlend(dwTempSrc16, *pDest, dwOpacity);
			}

			u += dudx;
			v += dvdx;
			pDest += destXPitch;
	        }

	        u0 += dudy;
	        v0 += dvdy;
	    }
    }
}

void BL_RotateBltOpacityColorFill565(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, LONG srcRectWidth,
	LONG srcRectHeight, DWORD dwNativeColorFill, DWORD dwOpacity)
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
    
    if (dwOpacity == OPACITY_50)
    {
    	    for (DWORD dy = y1; dy != y2; dy += (1<<6) )
	    {
	        WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
	        
	        DWORD u = u0;
	        DWORD v = v0;

	        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
	        {
			DWORD ix = u >> 16;	// src x
			DWORD iy = v >> 16;	// src y
			
			// Perform the pixel copy
			if (ix < srcRectWidth && iy < srcRectHeight)
				*pDest = pxutil565::AlphaBlendFast(dwNativeColorFill, *pDest);

			u += dudx;
			v += dvdx;
			pDest += destXPitch;
	        }

	        u0 += dudy;
	        v0 += dvdy;
	    }
    }
    else
    {
	    for (DWORD dy = y1; dy != y2; dy += (1<<6) )
	    {
	        WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
	        
	        DWORD u = u0;
	        DWORD v = v0;

	        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
	        {
			DWORD ix = u >> 16;	// src x
			DWORD iy = v >> 16;	// src y
			
			// Perform the pixel copy
			if (ix < srcRectWidth && iy < srcRectHeight)
				*pDest = pxutil565::AlphaBlend(dwNativeColorFill, *pDest, dwOpacity);

			u += dudx;
			v += dvdx;
			pDest += destXPitch;
	        }

	        u0 += dudy;
	        v0 += dvdy;
	    }
     }
}

void BL_RotateBltOpacityKeySrcColorFill565(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcRectWidth, LONG srcRectHeight, DWORD dwNativeColorKey, DWORD dwNativeColorFill,
	DWORD dwOpacity)
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
    
    if (dwOpacity == OPACITY_50)
    {
	    for (DWORD dy = y1; dy != y2; dy += (1<<6) )
	    {
	        WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
	        
	        DWORD u = u0;
	        DWORD v = v0;

	        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
	        {
			DWORD ix = u >> 16;	// src x
			DWORD iy = v >> 16;	// src y
			
			// Perform the pixel copy
			if (ix < srcRectWidth && iy < srcRectHeight)
			{
				WORD *pSrc = pxutil16bit::GetBuffPixels(src, srcXPitch, srcYPitch, ix, iy);
				
				if (*pSrc != dwNativeColorKey)
					*pDest = pxutil565::AlphaBlendFast(dwNativeColorFill, *pDest);
			}

			u += dudx;
			v += dvdx;
			pDest += destXPitch;
	        }

	        u0 += dudy;
	        v0 += dvdy;
	    }
    }
    else
    {
	    for (DWORD dy = y1; dy != y2; dy += (1<<6) )
	    {
	        WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1 >> 6, dy >> 6);
	        
	        DWORD u = u0;
	        DWORD v = v0;

	        for (DWORD dx = x1; dx != x2; dx += (1<<6) )
	        {
			DWORD ix = u >> 16;	// src x
			DWORD iy = v >> 16;	// src y
			
			// Perform the pixel copy
			if (ix < srcRectWidth && iy < srcRectHeight)
			{
				WORD *pSrc = pxutil16bit::GetBuffPixels(src, srcXPitch, srcYPitch, ix, iy);
				
				if (*pSrc != dwNativeColorKey)
					*pDest = pxutil565::AlphaBlend(dwNativeColorFill, *pDest, dwOpacity);
			}

			u += dudx;
			v += dvdx;
			pDest += destXPitch;
	        }

	        u0 += dudy;
	        v0 += dvdy;
	    }
      }
}
