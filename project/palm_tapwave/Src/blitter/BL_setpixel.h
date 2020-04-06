/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_SETPIXEL_H_
#define __BL_SETPIXEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 SetPixel Functions
**************************************************************/

void BL_SetPixel555(const void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, COLORREF dwColor);
	
void BL_SetPixelOpacity555(const void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, COLORREF dwColor, DWORD dwOpacity);

/**************************************************************
* RGB565 SetPixel Functions
**************************************************************/

void BL_SetPixel565(const void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, COLORREF dwColor);
	
void BL_SetPixelOpacity565(const void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, COLORREF dwColor, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_SETPIXEL_H_