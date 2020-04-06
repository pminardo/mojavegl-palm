/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_STRETCHBLTOPACITY_H_
#define __BL_STRETCHBLTOPACITY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 StretchBlt Functions
**************************************************************/

#define BL_StretchBltOpacity555\
	#error "Not yet implemented"
	
#define BL_StretchBltOpacityKeySrc555\
	#error "Not yet implemented"

#define BL_StretchBltOpacityKeySrcColorFill555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 StretchBlt Functions
**************************************************************/

void BL_StretchBltOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwOpacity);
	
void BL_StretchBltOpacityKeySrc565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwNativeColorKey, DWORD dwOpacity);
	
void BL_StretchBltOpacityKeySrcColorFill565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwNativeColorKey, DWORD dwNativeColorFill,
	DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_STRETCHBLTOPACITY_H_