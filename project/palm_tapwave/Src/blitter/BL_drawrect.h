/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_DRAWRECT_H_
#define __BL_DRAWRECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 DrawRect Functions
**************************************************************/

#define BL_DrawRect555\
	#error "Not yet supported"
	
#define BL_DrawRectOpacity555\
	#error "Not yet supported"

/**************************************************************
* RGB565 DrawRect Functions
**************************************************************/

void BL_DrawRect565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x1, LONG y1,
	LONG x2, LONG y2, COLORREF dwColor);
	
void BL_DrawRectOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x1, LONG y1,
	LONG x2, LONG y2, COLORREF dwColor, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_DRAWRECT_H_