/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_FILLPOLYGON_H_
#define __BL_FILLPOLYGON_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 FillPolygon Functions
**************************************************************/

#define BL_FillPolygon555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 FillPolygon Functions
**************************************************************/

void BL_FillPolygon565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, COLORREF dwColor,
	POINT *pPointArray, DWORD dwNumPoints, DWORD dwFlags, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_FILLPOLYGON_H_