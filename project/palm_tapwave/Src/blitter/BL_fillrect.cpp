/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_fillrect.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark -------- Generic 16-bit FillRect Functions -----------
#endif

// Performs a 16-bit FillRect() operation.
void _BL_FillRect16(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG x, LONG y,
	LONG rw, LONG rh, DWORD dwNativeColor)
{
	BOOL bDest32Aligned = pxutil::IsAligned32(x, y, destXPitch, destYPitch, destWidth, destHeight);
	
	DWORD dwNativeColor32 = dwNativeColor | dwNativeColor << 16;

	// Optimization: Clear entire dest, if possible
	if (x == 0 && y == 0 && rw == destWidth && rh == destHeight)
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD *pDest32;
		DWORD dwNumBytes;
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute number of bytes to fill
		dwNumBytes = destYPitch * destHeight * sizeof(WORD);
		
		// Use the unrolled 32-bit pixel filler, if possible
		// This loop fills 8 pixels at a time.
		pDest32 = (DWORD*)pDest16;
		while (dwNumBytes > 16)
		{
			*pDest32++ = dwNativeColor32;
			*pDest32++ = dwNativeColor32;
			*pDest32++ = dwNativeColor32;
			*pDest32++ = dwNativeColor32;
			dwNumBytes -= 16;
		}
		
		// Pick up any residual with the single pixel filler
		pDest16 = (WORD*)pDest32;
		
		// Fill individual pixels
		while (dwNumBytes != 0)
		{
			*pDest16++ = (WORD)dwNativeColor;
			dwNumBytes -= 2;
		}
	}
	// Use 32-bit blitter
	else if (bDest32Aligned &&
		(rw%2) == 0 && (rh%2) == 0)
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD *pDest32;
		DWORD dwDestOffset;
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute dest offset
		dwDestOffset = destYPitch - rw;
		
		// Init dest pointer
		pDest16 += x + (y * destYPitch);
		
		// Init for 32-bit blit
		pDest32 = (DWORD*)pDest16;
		dwDestOffset >>= 1;
		rw >>= 1;
		
		while (rh != 0) {
			
			#define BLIT_EXPR\
				*pDest32++ = dwNativeColor32;
			
			BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest32 += dwDestOffset;
			--rh;
		}
	}
	// Use 16-bit blitter
	else
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD dwDestOffset;
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute dest offset
		dwDestOffset = destYPitch - rw;
		
		// Init dest pointer
		pDest16 += x + (y * destYPitch);
		
		while (rh != 0) {
		
			#define BLIT_EXPR\
				*pDest16++ = (WORD)dwNativeColor;
			
			BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			--rh;
		}
	}
}

#ifdef _PRGM_SECT
#pragma mark -------- RGB565 FillRect Functions -----------
#endif

void BL_FillRectOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG x, LONG y,
	LONG rw, LONG rh, DWORD dwNativeColor, DWORD dwOpacity)
{
	BOOL bDest32Aligned = pxutil::IsAligned32(x, y, destXPitch, destYPitch, destWidth, destHeight);

	DWORD dwNativeColor32 = dwNativeColor | dwNativeColor << 16;

	// Use 32-bit blitter
	if (bDest32Aligned &&
		(rw%2) == 0 && (rh%2) == 0)
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD *pDest32;
		DWORD dwDestOffset;
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute dest offset
		dwDestOffset = destYPitch - rw;
		
		// Init dest pointer
		pDest16 += x + (y * destYPitch);
		
		// Init for 32-bit blit
		pDest32 = (DWORD*)pDest16;
		dwDestOffset >>= 1;
		rw >>= 1;
		
		// Optimize for fast 50% opacity
		if (dwOpacity == OPACITY_50)
		{
			while (rh != 0) {
				
				#define BLIT_EXPR\
					*pDest32++ = pxutil565::AlphaBlendFast32(dwNativeColor32, *pDest32);
				
				BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
				
				#undef BLIT_EXPR
			
				pDest32 += dwDestOffset;
				--rh;
			}
		}
		else
		{
			while (rh != 0) {
			
				#define BLIT_EXPR\
					*pDest32++ = pxutil565::AlphaBlend32(dwNativeColor32, *pDest32, dwOpacity);
				
				BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
				
				#undef BLIT_EXPR
			
				pDest32 += dwDestOffset;
				--rh;
			}
		}
	}
	// Use 16-bit blitter
	else
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD dwDestOffset;
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute dest offset
		dwDestOffset = destYPitch - rw;
		
		// Init dest pointer
		pDest16 += x + (y * destYPitch);
		
		// Optimize for fast 50% opacity
		if (dwOpacity == OPACITY_50)
		{
			while (rh != 0) {
				
				#define BLIT_EXPR\
					*pDest16++ = (WORD)pxutil565::AlphaBlendFast(dwNativeColor, *pDest16);
				
				BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
				
				#undef BLIT_EXPR
			
				pDest16 += dwDestOffset;
				--rh;
			}
		}
		// Standard alpha blending
		else
		{
			while (rh != 0) {
				
				#define BLIT_EXPR\
					*pDest16++ = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest16, dwOpacity);
				
				BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
				
				#undef BLIT_EXPR
			
				pDest16 += dwDestOffset;
				--rh;
			}
		}
	}
}