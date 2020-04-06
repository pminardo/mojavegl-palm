/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_DRAWPOLYLINE_H_
#define __BL_DRAWPOLYLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 DrawPolyline Functions
**************************************************************/

#define BL_DrawPolyline555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 DrawPolyline Functions
**************************************************************/

void BL_DrawPolyline565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, COLORREF dwColor,
	POINT *pPointArray, DWORD dwNumPoints, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif // __BL_DRAWPOLYLINE_H_