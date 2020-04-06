/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_DRAWFTBMP8_H_
#define __BL_DRAWFTBMP8_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 DrawFTBmp8 Functions
**************************************************************/

#define BL_DrawFTBmp8_555\
	#error "Not yet supported"
	
#define BL_DrawFTBmp8Opacity_555\
	#error "Not yet supported"

/**************************************************************
* RGB565 DrawFTBmp8 Functions
**************************************************************/

void BL_DrawFTBmp8_565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcPitch, LONG destX, LONG destY, LONG destWidth, LONG destHeight,
	LONG srcX, LONG srcY, LONG srcWidth, LONG srcHeight, DWORD dwNativeColor);
	
void BL_DrawFTBmp8Opacity_565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcPitch, LONG destX, LONG destY, LONG destWidth, LONG destHeight,
	LONG srcX, LONG srcY, LONG srcWidth, LONG srcHeight, DWORD dwNativeColor, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_DRAWFTBMP8_H_