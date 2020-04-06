/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_FILLRECT_H_
#define __BL_FILLRECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 FillRect Functions
**************************************************************/

#define BL_FillRect555			_BL_FillRect16
#define BL_FillRectOpacity555\
	#error "Not yet supported"

/**************************************************************
* RGB565 FillRect Functions
**************************************************************/

#define BL_FillRect565			_BL_FillRect16

void BL_FillRectOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG x, LONG y,
	LONG rw, LONG rh, DWORD dwNativeColor, DWORD dwOpacity);

/**************************************************************
* Generic 16-bit FillRect Functions
**************************************************************/

void _BL_FillRect16(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG x, LONG y,
	LONG rw, LONG rh, DWORD dwNativeColor);

#ifdef __cplusplus
}
#endif

#endif // __BL_FILLRECT_H_