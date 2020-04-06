/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_DRAWFTBMP1_H_
#define __BL_DRAWFTBMP1_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 DrawFTBmp1 Functions
**************************************************************/

#define BL_DrawFTBmp1_555\
	#error "Not yet supported"
	
#define BL_DrawFTBmp1Opacity_555\
	#error "Not yet supported"

/**************************************************************
* RGB565 DrawFTBmp1 Functions
**************************************************************/

void BL_DrawFTBmp1_565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcPitch, LONG destX, LONG destY, LONG destWidth, LONG destHeight,
	LONG srcX, LONG srcY, LONG srcWidth, LONG srcHeight, DWORD dwNativeColor);
	
void BL_DrawFTBmp1Opacity_565(void *dest, const void *src, LONG destXPitch, LONG destYPitch,
	LONG srcPitch, LONG destX, LONG destY, LONG destWidth, LONG destHeight,
	LONG srcX, LONG srcY, LONG srcWidth, LONG srcHeight, DWORD dwNativeColor, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_DRAWFTBMP1_H_