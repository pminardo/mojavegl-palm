/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_TILEBITBLT_H_
#define __BL_TILEBITBLT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 TileBitBlt Functions
**************************************************************/

#define BL_TileBitBlt555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 TileBitBlt Functions
**************************************************************/

void BL_TileBitBlt565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, LONG srcWidth, LONG srcHeight,
	RECT *pSrcRect, RECT *pClipRect, DWORD dwFlags,
	COLORREF dwColorKey, COLORREF dwFillColor);

#ifdef __cplusplus
}
#endif

#endif // __BL_TILEBITBLT_H_