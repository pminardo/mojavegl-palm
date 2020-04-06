/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_STRETCHALPHABLTSUBTRACTIVE_H_
#define __BL_STRETCHALPHABLTSUBTRACTIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 StretchAlphaBltSubtractive Functions
**************************************************************/

#define BL_StretchAlphaBltSubtractive555\
	#error "Not yet implemented"
	
#define BL_StretchAlphaBltSubtractiveOpacity555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 StretchAlphaBltSubtractive Functions
**************************************************************/

void BL_StretchAlphaBltSubtractive565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight, RECT *pDestRect,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect,
	LONG alphaXPitch, LONG alphaYPitch, RECT *pAlphaRect,
	RECT *pClipRect, DWORD dwFlags);
	
void BL_StretchAlphaBltSubtractiveOpacity565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight, RECT *pDestRect,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect,
	LONG alphaXPitch, LONG alphaYPitch, RECT *pAlphaRect,
	RECT *pClipRect, DWORD dwFlags, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_STRETCHALPHABLTSUBTRACTIVE_H_