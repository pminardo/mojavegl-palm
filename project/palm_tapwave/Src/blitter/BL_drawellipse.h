/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_DRAWELLIPSE_H_
#define __BL_DRAWELLIPSE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 DrawEllipse Functions
**************************************************************/

#define BL_DrawEllipse555\
	#error "Not yet implemented"

#define BL_DrawEllipseOpacity555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 DrawEllipse Functions
**************************************************************/

void BL_DrawEllipse565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, RECT *pRect,
	COLORREF dwColor);
	
void BL_DrawEllipseOpacity565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, RECT *pRect,
	COLORREF dwColor, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_DRAWELLIPSE_H_