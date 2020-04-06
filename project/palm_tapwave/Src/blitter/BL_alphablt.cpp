/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_alphablt.h"

/**************************************************************
* Constants
**************************************************************/

// Defines the maximum visible opacity
#define MAXALPHA		248

/**************************************************************
* Helper Functions
**************************************************************/

namespace AlphaBlt565 {

	inline
	DWORD BlendPixel(DWORD pixel, DWORD backpixel, DWORD alphapixel)
	{
		DWORD alpha = pxutil565::GetSurfaceAlphaValue(alphapixel);

		if(alpha)
		{
			if (alpha < MAXALPHA)
				return pxutil565::AlphaBlend(pixel, backpixel, alpha);

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
#pragma mark ------------ RGB565 AlphaBlt Functions --------------
#endif

// Performs a 16-bit alpha blit of source image to destination image.
void BL_AlphaBlt565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG srcXPitch, LONG srcYPitch,
	LONG alphaXPitch, LONG alphaYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, LONG alphaX, LONG alphaY,
	LONG alphaWidth, LONG alphaHeight)
{
	BOOL bDest32Aligned = pxutil::IsAligned32(destX, destY, destXPitch, destYPitch, destWidth, destHeight);
	BOOL bSrc32Aligned = pxutil::IsAligned32(srcX, srcY, srcXPitch, srcYPitch, srcWidth, srcHeight);
	BOOL bAlpha32Aligned = pxutil::IsAligned32(alphaX, alphaY, alphaXPitch, alphaYPitch, alphaWidth, alphaHeight);
	
	// 32-bit src & 32-bit dest blitter
	if (bDest32Aligned && (bSrc32Aligned && bAlpha32Aligned))
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		WORD *pAlpha16 = (WORD*)alpha;
		DWORD *pDest32, *pSrc32, *pAlpha32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwAlphaOffset;
		DWORD dwTempSrc32, dwTempAlpha32, dwTempDest32;
		
		// Optimize dest, src, and alpha pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		pxutil::OptimizePitch(pAlpha16, alphaWidth, alphaHeight, alphaXPitch, alphaYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		dwAlphaOffset = alphaYPitch - srcWidth;
		
		// Init dest, src, and alpha pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		pAlpha16 += alphaX + (alphaY * alphaYPitch);
		
		// Init for 32-bit blit
		pDest32 = (DWORD*)pDest16;
		pSrc32 = (DWORD*)pSrc16;
		pAlpha32 = (DWORD*)pAlpha16;
		dwDestOffset >>= 1;
		dwSrcOffset >>= 1;
		dwAlphaOffset >>= 1;
		srcWidth >>= 1;
		alphaWidth >>= 1;
		
		while (srcHeight != 0) {
		
			#define BLIT_EXPR\
			{\
			dwTempSrc32 = *pSrc32++;\
			dwTempAlpha32 = *pAlpha32++;\
			dwTempDest32 = *pDest32;\
			*pDest32++ = AlphaBlt565::BlendPixel(dwTempSrc32 & 0x0000FFFFL,\
							dwTempDest32 & 0x0000FFFFL,\
							dwTempAlpha32 & 0x0000FFFFL) | \
						(AlphaBlt565::BlendPixel(dwTempSrc32 >> 16,\
							dwTempDest32 >> 16,\
							dwTempAlpha32 >> 16) << 16);\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest32 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			pAlpha32 += dwAlphaOffset;
			--srcHeight;
		}
	}
	// 32-bit src, 32-bit mask & 16-bit dest blitter
	else if (!bDest32Aligned && (bSrc32Aligned && bAlpha32Aligned))
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		WORD *pAlpha16 = (WORD*)alpha;
		DWORD *pSrc32;
		DWORD *pAlpha32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwAlphaOffset;
		DWORD dwTempSrc32, dwTempAlpha32;
		
		// Optimize dest, src, and alpha pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		pxutil::OptimizePitch(pAlpha16, alphaWidth, alphaHeight, alphaXPitch, alphaYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		dwAlphaOffset = alphaYPitch - srcWidth;
		
		// Init dest, src, and alpha pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		pAlpha16 += alphaX + (alphaY * alphaYPitch);
		
		// Init for 32-bit blit (source only)
		pSrc32 = (DWORD*)pSrc16;
		pAlpha32 = (DWORD*)pAlpha16;
		dwSrcOffset >>= 1;
		dwAlphaOffset >>= 1;
		srcWidth >>= 1;
		alphaWidth >>= 1;
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
			{\
				/* Read 2 pixels from source and alpha images */ \
				dwTempSrc32 = *pSrc32++;\
				dwTempAlpha32 = *pAlpha32++;\
			\
				/* Write 2 pixels on destination image */ \
			\
				/* Write first pixel */ \
				*pDest16++ = (WORD)AlphaBlt565::BlendPixel(dwTempSrc32 & 0x0000FFFFL,\
					*pDest16, dwTempAlpha32 & 0x0000FFFFL);\
			\
				/* Write second pixel */ \
				*pDest16++ = (WORD)AlphaBlt565::BlendPixel(dwTempSrc32 >> 16,\
					*pDest16, dwTempAlpha32 >> 16);\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			pAlpha32 += dwAlphaOffset;
			--srcHeight;
		}
	}
	// Standard 16-bit blitter
	else
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		WORD *pAlpha16 = (WORD*)alpha;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwAlphaOffset;
		
		// Optimize dest, src, and alpha pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		pxutil::OptimizePitch(pAlpha16, alphaWidth, alphaHeight, alphaXPitch, alphaYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		dwAlphaOffset = alphaYPitch - srcWidth;
		
		// Init dest, src, and alpha pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		pAlpha16 += alphaX + (alphaY * alphaYPitch);
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
				*pDest16++ = (WORD)AlphaBlt565::BlendPixel(*pSrc16++, *pDest16, *pAlpha16++);
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc16 += dwSrcOffset;
			pAlpha16 += dwAlphaOffset;
			--srcHeight;
		}
	}
}

// Performs a 16-bit alpha blit of source image to destination image.
void BL_AlphaBltOpacity565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG srcXPitch, LONG srcYPitch,
	LONG alphaXPitch, LONG alphaYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, LONG alphaX, LONG alphaY,
	LONG alphaWidth, LONG alphaHeight, DWORD dwOpacity)
{
	BOOL bDest32Aligned = pxutil::IsAligned32(destX, destY, destXPitch, destYPitch, destWidth, destHeight);
	BOOL bSrc32Aligned = pxutil::IsAligned32(srcX, srcY, srcXPitch, srcYPitch, srcWidth, srcHeight);
	BOOL bAlpha32Aligned = pxutil::IsAligned32(alphaX, alphaY, alphaXPitch, alphaYPitch, alphaWidth, alphaHeight);

	// 32-bit src & 32-bit dest blitter
	if (bDest32Aligned && (bSrc32Aligned && bAlpha32Aligned))
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		WORD *pAlpha16 = (WORD*)alpha;
		DWORD *pDest32, *pSrc32, *pAlpha32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwAlphaOffset;
		DWORD dwTempSrc32, dwTempAlpha32, dwTempDest32;
		
		// Optimize dest, src, and alpha pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		pxutil::OptimizePitch(pAlpha16, alphaWidth, alphaHeight, alphaXPitch, alphaYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		dwAlphaOffset = alphaYPitch - srcWidth;
		
		// Init dest, src, and alpha pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		pAlpha16 += alphaX + (alphaY * alphaYPitch);
		
		// Init for 32-bit blit
		pDest32 = (DWORD*)pDest16;
		pSrc32 = (DWORD*)pSrc16;
		pAlpha32 = (DWORD*)pAlpha16;
		dwDestOffset >>= 1;
		dwSrcOffset >>= 1;
		dwAlphaOffset >>= 1;
		srcWidth >>= 1;
		alphaWidth >>= 1;
		
		while (srcHeight != 0) {
		
			#define BLIT_EXPR\
			{\
			dwTempSrc32 = *pSrc32++;\
			dwTempAlpha32 = *pAlpha32++;\
			dwTempDest32 = *pDest32;\
			*pDest32++ = AlphaBlt565::BlendPixelOpacity(dwTempSrc32 & 0x0000FFFFL,\
							dwTempDest32 & 0x0000FFFFL,\
							dwTempAlpha32 & 0x0000FFFFL,\
							dwOpacity) | \
						(AlphaBlt565::BlendPixelOpacity(dwTempSrc32 >> 16,\
							dwTempDest32 >> 16,\
							dwTempAlpha32 >> 16,\
							dwOpacity) << 16);\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest32 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			pAlpha32 += dwAlphaOffset;
			--srcHeight;
		}
	}
	// 32-bit src, 32-bit mask & 16-bit dest blitter
	else if (!bDest32Aligned && (bSrc32Aligned && bAlpha32Aligned))
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		WORD *pAlpha16 = (WORD*)alpha;
		DWORD *pSrc32;
		DWORD *pAlpha32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwAlphaOffset;
		DWORD dwTempSrc32, dwTempAlpha32;
		
		// Optimize dest, src, and alpha pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		pxutil::OptimizePitch(pAlpha16, alphaWidth, alphaHeight, alphaXPitch, alphaYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		dwAlphaOffset = alphaYPitch - srcWidth;
		
		// Init dest, src, and alpha pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		pAlpha16 += alphaX + (alphaY * alphaYPitch);
		
		// Init for 32-bit blit (source only)
		pSrc32 = (DWORD*)pSrc16;
		pAlpha32 = (DWORD*)pAlpha16;
		dwSrcOffset >>= 1;
		dwAlphaOffset >>= 1;
		srcWidth >>= 1;
		alphaWidth >>= 1;
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
			{\
				/* Read 2 pixels from source and alpha images */ \
				dwTempSrc32 = *pSrc32++;\
				dwTempAlpha32 = *pAlpha32++;\
			\
				/* Write 2 pixels on destination image */ \
			\
				/* Write first pixel */ \
				*pDest16++ = (WORD)AlphaBlt565::BlendPixelOpacity(dwTempSrc32 & 0x0000FFFFL,\
					*pDest16, dwTempAlpha32 & 0x0000FFFFL, dwOpacity);\
			\
				/* Write second pixel */ \
				*pDest16++ = (WORD)AlphaBlt565::BlendPixelOpacity(dwTempSrc32 >> 16,\
					*pDest16, dwTempAlpha32 >> 16, dwOpacity);\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			pAlpha32 += dwAlphaOffset;
			--srcHeight;
		}
	}
	// Standard 16-bit blitter
	else
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		WORD *pAlpha16 = (WORD*)alpha;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwAlphaOffset;
		
		// Optimize dest, src, and alpha pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		pxutil::OptimizePitch(pAlpha16, alphaWidth, alphaHeight, alphaXPitch, alphaYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		dwAlphaOffset = alphaYPitch - srcWidth;
		
		// Init dest, src, and alpha pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		pAlpha16 += alphaX + (alphaY * alphaYPitch);
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
				*pDest16++ = (WORD)AlphaBlt565::BlendPixelOpacity(*pSrc16++, *pDest16, *pAlpha16++, dwOpacity);
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc16 += dwSrcOffset;
			pAlpha16 += dwAlphaOffset;
			--srcHeight;
		}
	}
}