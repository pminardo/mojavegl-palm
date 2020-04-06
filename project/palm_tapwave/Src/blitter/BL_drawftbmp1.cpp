/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_drawftbmp1.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 DrawFTBmp1 Functions --------------
#endif

void BL_DrawFTBmp1_565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcPitch, LONG destX, LONG destY, LONG destWidth, LONG destHeight,
	LONG srcX, LONG srcY, LONG srcWidth, LONG srcHeight, DWORD dwNativeColor)
{
	WORD *pDest16 = (WORD*)dest;
	BYTE *pSrc1 = (BYTE*)src;
	DWORD dwDestOffset;
	DWORD dwSrcRowByteFirst;
	DWORD dwSrcCpyBytesPerRow, dwSrcXBitOffset;
	
	dwDestOffset = destYPitch - srcWidth;
	
	// Figure out which byte corresponds to srcX
	dwSrcRowByteFirst = (srcX/8);
	dwSrcXBitOffset = srcX%8;
	
	// Figure out which byte corresponds to srcWidth
	dwSrcCpyBytesPerRow = (srcWidth/8);
	if (srcWidth%8) ++dwSrcCpyBytesPerRow;
	
	// Init dest and src pointers
	pDest16 += destX + (destY * destYPitch);
	pSrc1 += dwSrcRowByteFirst + (srcY * srcPitch);

	while (srcHeight != 0) {
	
		BYTE *src = pSrc1;
		DWORD i = dwSrcXBitOffset;
		DWORD dwNumPixelsToCpy = srcWidth;
		
		// Process a row of pixels
		while (dwNumPixelsToCpy != 0)
		{
			// Process 8 pixels at a time
			while (i < 8)
			{
				// If the pixel is set in the source, set it in the destination
				if ((*src >> (7-i)) & 0x01)
					*pDest16 = dwNativeColor;
			
				pDest16 += destXPitch;
				--dwNumPixelsToCpy;
				
				// Are we out of pixels?
				if (!dwNumPixelsToCpy)
					break;

				++i;
			}
			
			// Advance to next byte (8 pixels)
			++src;
			i = 0;
		}
	
		pDest16 += dwDestOffset;
		pSrc1 += srcPitch;
		--srcHeight;
	}
}

void BL_DrawFTBmp1Opacity_565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcPitch, LONG destX, LONG destY, LONG destWidth, LONG destHeight,
	LONG srcX, LONG srcY, LONG srcWidth, LONG srcHeight, DWORD dwNativeColor, DWORD dwOpacity)
{
	WORD *pDest16 = (WORD*)dest;
	BYTE *pSrc1 = (BYTE*)src;
	DWORD dwDestOffset;
	DWORD dwSrcRowByteFirst;
	DWORD dwSrcCpyBytesPerRow, dwSrcXBitOffset;
	
	dwDestOffset = destYPitch - srcWidth;
	
	// Figure out which byte corresponds to srcX
	dwSrcRowByteFirst = (srcX/8);
	dwSrcXBitOffset = srcX%8;
	
	// Figure out which byte corresponds to srcWidth
	dwSrcCpyBytesPerRow = (srcWidth/8);
	if (srcWidth%8) ++dwSrcCpyBytesPerRow;
	
	// Init dest and src pointers
	pDest16 += destX + (destY * destYPitch);
	pSrc1 += dwSrcRowByteFirst + (srcY * srcPitch);
	
	// Optimize for 50% opacity
	if (dwOpacity == OPACITY_50)
	{
		while (srcHeight != 0) {
		
			BYTE *src = pSrc1;
			DWORD i = dwSrcXBitOffset;
			DWORD dwNumPixelsToCpy = srcWidth;
			
			// Process a row of pixels
			while (dwNumPixelsToCpy != 0)
			{
				// Process 8 pixels at a time
				while (i < 8)
				{
					// If the pixel is set in the source, set it in the destination
					if ((*src >> (7-i)) & 0x01)
						*pDest16 = pxutil565::AlphaBlendFast(dwNativeColor, *pDest16);
				
					pDest16 += destXPitch;
					--dwNumPixelsToCpy;
					
					// Are we out of pixels?
					if (!dwNumPixelsToCpy)
						break;

					++i;
				}
				
				// Advance to next byte (8 pixels)
				++src;
				i = 0;
			}
		
			pDest16 += dwDestOffset;
			pSrc1 += srcPitch;
			--srcHeight;
		}
	}
	else
	{
		while (srcHeight != 0) {
		
			BYTE *src = pSrc1;
			DWORD i = dwSrcXBitOffset;
			DWORD dwNumPixelsToCpy = srcWidth;
			
			// Process a row of pixels
			while (dwNumPixelsToCpy != 0)
			{
				// Process 8 pixels at a time
				while (i < 8)
				{
					// If the pixel is set in the source, set it in the destination
					if ((*src >> (7-i)) & 0x01)
						*pDest16 = pxutil565::AlphaBlend(dwNativeColor, *pDest16, dwOpacity);
				
					pDest16 += destXPitch;
					--dwNumPixelsToCpy;
					
					// Are we out of pixels?
					if (!dwNumPixelsToCpy)
						break;

					++i;
				}
				
				// Advance to next byte (8 pixels)
				++src;
				i = 0;
			}
		
			pDest16 += dwDestOffset;
			pSrc1 += srcPitch;
			--srcHeight;
		}
	}
}