/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_STRETCHALPHABLT_H_
#define __BL_STRETCHALPHABLT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 StretchAlphaBlt Functions
**************************************************************/

#define BL_StretchAlphaBlt555\
	#error "Not yet implemented"
	
#define BL_StretchAlphaBltOpacity555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 StretchAlphaBlt Functions
**************************************************************/

void BL_StretchAlphaBlt565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight, RECT *pDestRect,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect,
	LONG alphaXPitch, LONG alphaYPitch, RECT *pAlphaRect,
	RECT *pClipRect, DWORD dwFlags);
	
void BL_StretchAlphaBltOpacity565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight, RECT *pDestRect,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect,
	LONG alphaXPitch, LONG alphaYPitch, RECT *pAlphaRect,
	RECT *pClipRect, DWORD dwFlags, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_STRETCHALPHABLT_H_