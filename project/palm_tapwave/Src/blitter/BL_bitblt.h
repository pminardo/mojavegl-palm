/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_BITBLT_H_
#define __BL_BITBLT_H_

/**************************************************************
* RGB555 BitBlt Functions
**************************************************************/

// RGB555 is easy - just map to the generic 16-bit blitters.

#define BL_BitBlt555					_BL_BitBlt16
#define BL_BitBltKeySrc555				_BL_BitBltKeySrc16
#define BL_BitBltKeySrcColorFill555		_BL_BitBltKeySrcColorFill16

/**************************************************************
* RGB565 BitBlt Functions
**************************************************************/

// RGB565 is easy - just map to the generic 16-bit blitters.

#define BL_BitBlt565					_BL_BitBlt16
#define BL_BitBltKeySrc565				_BL_BitBltKeySrc16
#define BL_BitBltKeySrcColorFill565		_BL_BitBltKeySrcColorFill16

/**************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* Generic 16-bit BitBlt Functions
**************************************************************/

// Performs a 16-bit blit of source image to destination image.
void _BL_BitBlt16(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight);
	
// Performs a 16-bit blit of source image to destination image
// using the specified color key.
void _BL_BitBltKeySrc16(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, DWORD dwNativeColorKey);
	
// Performs a 16-bit blit of source image to destination image
// using the specified color key. Source pixels are drawn as
// the colorfill color. 
void _BL_BitBltKeySrcColorFill16(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcXPitch, LONG srcYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, DWORD dwNativeColorKey,
	DWORD dwNativeColorFill);

#ifdef __cplusplus
}
#endif

#endif // __BL_BITBLT_H_