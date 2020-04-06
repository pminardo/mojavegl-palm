/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_TILEBITBLTOPACITY_H_
#define __BL_TILEBITBLTOPACITY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 TileBitBltOpacity Functions
**************************************************************/

#define BL_TileBitBltOpacity555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 TileBitBltOpacity Functions
**************************************************************/

void BL_TileBitBltOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, LONG srcWidth, LONG srcHeight,
	RECT *pSrcRect, RECT *pClipRect, DWORD dwFlags,
	COLORREF dwColorKey, COLORREF dwFillColor, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_TILEBITBLTOPACITY_H_