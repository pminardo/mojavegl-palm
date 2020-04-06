/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_FILLELLIPSE_H_
#define __BL_FILLELLIPSE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 FillEllipse Functions
**************************************************************/

#define BL_FillEllipse555\
	#error "Not yet implemented"

#define BL_FillEllipseOpacity555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 FillEllipse Functions
**************************************************************/

void BL_FillEllipse565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, RECT *pRect,
	COLORREF dwColor);
	
void BL_FillEllipseOpacity565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, RECT *pRect,
	COLORREF dwColor, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_FILLELLIPSE_H_