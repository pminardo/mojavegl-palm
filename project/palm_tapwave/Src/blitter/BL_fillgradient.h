/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_FILLGRADIENT_H_
#define __BL_FILLGRADIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 FillGradient Functions
**************************************************************/

#define BL_FillGradient555\
	#error "Not yet supported"
	
#define BL_FillGradientDither555\
	#error "Not yet supported"

/**************************************************************
* RGB565 FillGradient Functions
**************************************************************/

void BL_FillGradient565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG dx, LONG dy,
	LONG dx2, LONG dy2, COLORREF dwColor1, COLORREF dwColor2,
	DWORD dwFlags, DWORD dwOpacity);
	
void BL_FillGradientDither565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG dx, LONG dy,
	LONG dx2, LONG dy2, COLORREF dwColor1, COLORREF dwColor2,
	DWORD dwFlags, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_FILLGRADIENT_H_