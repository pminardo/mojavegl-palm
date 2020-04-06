/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_BITBLTOPACITY_H_
#define __BL_BITBLTOPACITY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 BitBltOpacity Functions
**************************************************************/

#define BL_BitBltOpacity555\
	#error "Not yet supported"
	
#define BL_BitBltOpacityKeySrc555\
	#error "Not yet supported"
	
#define BL_BitBltOpacityKeySrcColorFill555\
	#error "Not yet supported"

/**************************************************************
* RGB565 BitBltOpacity Functions
**************************************************************/

void BL_BitBltOpacity565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, DWORD dwOpacity);
	
void BL_BitBltOpacityKeySrc565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, DWORD dwNativeColorKey, DWORD dwOpacity);

void BL_BitBltOpacityKeySrcColorFill565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, DWORD dwNativeColorKey,
	DWORD dwNativeColorFill, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_BITBLTOPACITY_H_