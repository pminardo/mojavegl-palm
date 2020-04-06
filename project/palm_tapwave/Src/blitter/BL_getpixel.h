/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_GETPIXEL_H_
#define __BL_GETPIXEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 GetPixel Functions
**************************************************************/

COLORREF BL_GetPixel555(const void *src, LONG srcXPitch,
	LONG srcYPitch, LONG srcX, LONG srcY);

/**************************************************************
* RGB565 GetPixel Functions
**************************************************************/

COLORREF BL_GetPixel565(const void *src, LONG srcXPitch,
	LONG srcYPitch, LONG srcX, LONG srcY);

#ifdef __cplusplus
}
#endif

#endif // __BL_GETPIXEL_H_