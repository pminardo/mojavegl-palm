/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_STRETCHBLT_H_
#define __BL_STRETCHBLT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 StretchBlt Functions
**************************************************************/

#define BL_StretchBlt555					_BL_StretchBlt16

#define BL_StretchBltKeySrc555			_BL_StretchBltKeySrc16

#define BL_StretchBltKeySrcColorFill555		_BL_StretchBltKeySrcColorFill16

/**************************************************************
* RGB565 StretchBlt Functions
**************************************************************/

#define BL_StretchBlt565					_BL_StretchBlt16

#define BL_StretchBltKeySrc565			_BL_StretchBltKeySrc16

#define BL_StretchBltKeySrcColorFill565		_BL_StretchBltKeySrcColorFill16
	
/**************************************************************
* Generic 16-bit StretchBlt Functions
**************************************************************/	
	
void _BL_StretchBlt16(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags);
	
void _BL_StretchBltKeySrc16(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwNativeColorKey);
	
void _BL_StretchBltKeySrcColorFill16(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pDestRect, void *src,
	LONG srcXPitch, LONG srcYPitch, RECT *pSrcRect, RECT *pClipRect,
	DWORD dwFlags, DWORD dwNativeColorKey, DWORD dwNativeColorFill);

#ifdef __cplusplus
}
#endif

#endif // __BL_STRETCHBLT_H_