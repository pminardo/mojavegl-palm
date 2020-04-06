/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_DRAWBEZIER_H_
#define __BL_DRAWBEZIER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 DrawBezier Functions
**************************************************************/

#define BL_DrawBezier555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 DrawBezier Functions
**************************************************************/

void BL_DrawBezier565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, COLORREF dwColor,
	POINT *pPoints /*ptr to 4 element POINT array*/, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif // __BL_DRAWBEZIER_H_