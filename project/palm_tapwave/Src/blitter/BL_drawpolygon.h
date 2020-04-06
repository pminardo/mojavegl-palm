/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_DRAWPOLYGON_H_
#define __BL_DRAWPOLYGON_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 DrawPolygon Functions
**************************************************************/

#define BL_DrawPolygon555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 DrawPolygon Functions
**************************************************************/

void BL_DrawPolygon565 (void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, COLORREF dwColor,
	POINT *pPointArray, DWORD dwNumPoints, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif // __BL_DRAWPOLYGON_H_