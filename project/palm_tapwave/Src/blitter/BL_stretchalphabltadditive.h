/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_STRETCHALPHABLTADDITIVE_H_
#define __BL_STRETCHALPHABLTADDITIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 StretchAlphaBltAdditive Functions
**************************************************************/

#define BL_StretchAlphaBltAdditive555\
	#error "Not yet implemented"
	
#define BL_StretchAlphaBltAdditiveOpacity555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 StretchAlphaBltAdditive Functions
**************************************************************/

void BL_StretchAlphaBltAdditive565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight, RECT *pDestRect,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect,
	LONG alphaXPitch, LONG alphaYPitch, RECT *pAlphaRect,
	RECT *pClipRect, DWORD dwFlags);
	
void BL_StretchAlphaBltAdditiveOpacity565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight, RECT *pDestRect,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect,
	LONG alphaXPitch, LONG alphaYPitch, RECT *pAlphaRect,
	RECT *pClipRect, DWORD dwFlags, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_STRETCHALPHABLTADDITIVE_H_