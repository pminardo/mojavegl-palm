/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_mem.h"
#include "BL_bitblt.h"

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ Generic 16-bit BitBlt Functions --------------
#endif

// Performs a 16-bit blit of source image to destination image.
void _BL_BitBlt16(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight)
{
	BOOL bDest32Aligned = pxutil::IsAligned32(destX, destY, destXPitch, destYPitch, destWidth, destHeight);
	BOOL bSrc32Aligned = pxutil::IsAligned32(srcX, srcY, srcXPitch, srcYPitch, srcWidth, srcHeight);

	// Optimize away blit using memcpy, if possible
	if (destX == 0 && destY == 0 &&
		srcX == 0 && srcY == 0 &&
		destXPitch == srcXPitch &&
		destYPitch == srcYPitch &&
		destWidth == srcWidth &&
		destHeight == srcHeight)
	{
		DWORD dwSize = srcHeight * srcYPitch * sizeof(WORD);
		
		// Perform the blit for the entire surface
		BL_memcpy (dest, src, dwSize);
	}
	// 32-bit src & 32-bit dest blitter
	else if (bDest32Aligned && bSrc32Aligned)
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD *pDest32, *pSrc32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		
		// Optimize src & dest pitch.
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		
		// Init for 32-bit blit
		pDest32 = (DWORD*)pDest16;
		pSrc32 = (DWORD*)pSrc16;
		dwDestOffset >>= 1;
		dwSrcOffset >>= 1;
		srcWidth >>= 1;
	
		while (srcHeight != 0) {
		
			#define BLIT_EXPR\
				*pDest32++ = *pSrc32++;
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
			
			pDest32 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			--srcHeight;
		}
	}
	// 32-bit src & 16-bit dest blitter
	else if (bSrc32Aligned)
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD *pSrc32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwTempSrc32;
		
		// Optimize src & dest pitch.
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		
		// Init for 32-bit blit (source only)
		pSrc32 = (DWORD*)pSrc16;
		dwSrcOffset >>= 1;
		srcWidth >>= 1;
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
			{\
				/* Read 2 pixels from source image */ \
				dwTempSrc32 = *pSrc32++;\
			\
				/* Write 2 pixels on destination image */ \
				*pDest16++ = (WORD)(dwTempSrc32 & 0x0000FFFFL);\
				*pDest16++ = (WORD)(dwTempSrc32 >> 16);\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			--srcHeight;
		}
	}
	// Standard 16-bit blitter
	else
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		
		// Optimize src & dest pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
	
		while (srcHeight != 0) {
		
			#define BLIT_EXPR\
			{\
				*pDest16++ = *pSrc16++;\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc16 += dwSrcOffset;
			--srcHeight;
		}
	}
}

// Performs a 16-bit blit of source image to destination image
// using the specified color key.
void _BL_BitBltKeySrc16(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, DWORD dwNativeColorKey)
{
	BOOL bDest32Aligned = pxutil::IsAligned32(destX, destY, destXPitch, destYPitch, destWidth, destHeight);
	BOOL bSrc32Aligned = pxutil::IsAligned32(srcX, srcY, srcXPitch, srcYPitch, srcWidth, srcHeight);

	DWORD dwNativeColorKey32 = dwNativeColorKey | dwNativeColorKey << 16;

	// 32-bit src & 32-bit dest blitter
	if (bDest32Aligned && bSrc32Aligned)
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD *pDest32, *pSrc32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwTempSrc32, dwResult32;
		
		// Optimize src & dest pitch.
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		
		// Init for 32-bit blit
		pDest32 = (DWORD*)pDest16;
		pSrc32 = (DWORD*)pSrc16;
		dwDestOffset >>= 1;
		dwSrcOffset >>= 1;
		srcWidth >>= 1;
	
		while (srcHeight != 0) {

			#define BLIT_EXPR\
			{\
				dwTempSrc32 = *pSrc32++;\
			\
				/* Copy pixels if they don't match the 32-bit colorkey */ \
				if (dwTempSrc32 != dwNativeColorKey32)\
				{\
					dwResult32 = *pDest32;\
				\
					/* See if either of the two pixels is equal to the color key value */ \
					if ((dwTempSrc32 & 0x0000FFFFL) != dwNativeColorKey)\
					{\
						dwResult32 &= 0xFFFF0000L;	/*clear low-word to 0*/ \
						dwResult32 |= dwTempSrc32 & 0x0000FFFFL;\
					}\
					if ((dwTempSrc32 >> 16) != dwNativeColorKey)\
					{\
						dwResult32 &= 0x0000FFFFL;	/*clear hi-word to 0*/ \
						dwResult32 |= dwTempSrc32 & 0xFFFF0000L; \
					}\
					\
					*pDest32 = dwResult32;\
				}\
				\
				++pDest32;	/* skip to next pair of pixels */ \
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest32 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			--srcHeight;
		}
	}
	// 32-bit src & 16-bit dest blitter
	else if (bSrc32Aligned)
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD *pSrc32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwTempSrc32;
		
		// Optimize src & dest pitch.
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		
		// Init for 32-bit blit (source only)
		pSrc32 = (DWORD*)pSrc16;
		dwSrcOffset >>= 1;
		srcWidth >>= 1;
	
		while (srcHeight != 0) {

			#define BLIT_EXPR\
			{\
				/* Read 2 pixels from source image */ \
				dwTempSrc32 = *pSrc32++; \
				\
				/* Copy pixels if they don't match the 32-bit colorkey */ \
				if (dwTempSrc32 != dwNativeColorKey32)\
				{\
					/* Write first pixel on destination image */ \
					if ((dwTempSrc32 & 0x0000FFFFL) != dwNativeColorKey)\
						*pDest16 = (WORD)(dwTempSrc32 & 0x0000FFFFL);\
					\
					/* Skip to next pixel */ \
					++pDest16;\
					\
					/* Write second pixel on destination image */ \
					if ((dwTempSrc32 >> 16) != dwNativeColorKey)\
						*pDest16 = (WORD)(dwTempSrc32 >> 16);\
					\
					/* Skip to next pixel */ \
					++pDest16;\
				}\
				else\
					pDest16 += 2;\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			--srcHeight;
		}
	}
	// Standard 16-bit blitter
	else
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwTempSrc16;
		
		// Optimize src & dest pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
			{\
				/* Copy pixel if it doesn't match the colorkey */ \
				if ((dwTempSrc16 = *pSrc16) != dwNativeColorKey)\
					*pDest16 = (WORD)dwTempSrc16;\
				\
				++pSrc16;\
				++pDest16;\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc16 += dwSrcOffset;
			--srcHeight;
		}
	}
}

// Performs a 16-bit blit of source image to destination image
// using the specified color key. Source pixels are drawn as
// the colorfill color. 
void _BL_BitBltKeySrcColorFill16(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, DWORD dwNativeColorKey,
	DWORD dwNativeColorFill)
{
	BOOL bDest32Aligned = pxutil::IsAligned32(destX, destY, destXPitch, destYPitch, destWidth, destHeight);
	BOOL bSrc32Aligned = pxutil::IsAligned32(srcX, srcY, srcXPitch, srcYPitch, srcWidth, srcHeight);

	DWORD dwNativeColorKey32 = dwNativeColorKey | dwNativeColorKey << 16;
	DWORD dwNativeColorFill32 = dwNativeColorFill | dwNativeColorFill << 16;

	// 32-bit src & 32-bit dest blitter
	if (bDest32Aligned && bSrc32Aligned)
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD *pDest32, *pSrc32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwTempSrc32, dwResult32;
		
		// Optimize src & dest pitch.
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		
		// Init for 32-bit blit
		pDest32 = (DWORD*)pDest16;
		pSrc32 = (DWORD*)pSrc16;
		dwDestOffset >>= 1;
		dwSrcOffset >>= 1;
		srcWidth >>= 1;
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
			{\
				dwTempSrc32 = *pSrc32++;\
			\
				/* Copy pixels if they don't match the 32-bit colorkey */ \
				if (dwTempSrc32 != dwNativeColorKey32)\
				{\
					dwResult32 = *pDest32;\
				\
					/* See if either of the two pixels is equal to the color key value */ \
					if ((dwTempSrc32 & 0x0000FFFFL) != dwNativeColorKey)\
					{\
						dwResult32 &= 0xFFFF0000L;	/* clear low-word to 0 */ \
						dwResult32 |= dwNativeColorFill32 & 0x0000FFFFL;\
					}\
					if ((dwTempSrc32 >> 16) != dwNativeColorKey)\
					{\
						dwResult32 &= 0x0000FFFFL;	/* clear hi-word to 0 */ \
						dwResult32 |= dwNativeColorFill32 & 0xFFFF0000L;\
					}\
					\
					*pDest32 = dwResult32;\
				}\
			\
				++pDest32;	/* skip to next pair of pixels */ \
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest32 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			--srcHeight;
		}
	}
	// 32-bit src & 16-bit dest blitter
	else if (bSrc32Aligned)
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD *pSrc32;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		DWORD dwTempSrc32;
		
		// Optimize src & dest pitch.
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
		
		// Init for 32-bit blit (source only)
		pSrc32 = (DWORD*)pSrc16;
		dwSrcOffset >>= 1;
		srcWidth >>= 1;
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
			{\
				/* Read 2 pixels from source image */ \
				dwTempSrc32 = *pSrc32++;\
			\
				/* Copy pixels if they don't match the 32-bit colorkey */ \
				if (dwTempSrc32 != dwNativeColorKey32)\
				{\
					/* Write first pixel on destination image */ \
					if ((dwTempSrc32 & 0x0000FFFFL) != dwNativeColorKey)\
						*pDest16 = (WORD)dwNativeColorFill;\
				\
					/* Skip to next pixel */ \
					++pDest16;\
				\
					/* Write second pixel on destination image */ \
					if ((dwTempSrc32 >> 16) != dwNativeColorKey)\
						*pDest16 = (WORD)dwNativeColorFill;\
				\
					/* Skip to next pixel */ \
					++pDest16;\
				}\
				else\
					pDest16 += 2;\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc32 += dwSrcOffset;
			--srcHeight;
		}
	}
	// Standard 16-bit blitter
	else
	{
		WORD *pDest16 = (WORD*)dest;
		WORD *pSrc16 = (WORD*)src;
		DWORD dwDestOffset;
		DWORD dwSrcOffset;
		
		// Optimize src & dest pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		pxutil::OptimizePitch(pSrc16, srcWidth, srcHeight, srcXPitch, srcYPitch);
		
		dwDestOffset = destYPitch - srcWidth;
		dwSrcOffset = srcYPitch - srcWidth;
		
		// Init dest and src pointers
		pDest16 += destX + (destY * destYPitch);
		pSrc16 += srcX + (srcY * srcYPitch);
	
		while (srcHeight != 0) {
			
			#define BLIT_EXPR\
			{\
				/* Copy pixel if it doesn't match the colorkey */ \
				if (*pSrc16 != dwNativeColorKey)\
					*pDest16 = (WORD)dwNativeColorFill;\
				\
				++pSrc16;\
				++pDest16;\
			}
			
			BLITTER_DUFF_DEVICE(srcWidth, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			pSrc16 += dwSrcOffset;
			--srcHeight;
		}
	}
}