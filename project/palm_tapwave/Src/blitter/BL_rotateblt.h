/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_ROTATEBLT_H_
#define __BL_ROTATEBLT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 RotateBlt Functions
**************************************************************/

#define BL_RotateBlt555				_BL_RotateBlt16
	
#define BL_RotateBltKeySrc555		_BL_RotateBltKeySrc16
	
#define BL_RotateBltColorFill555		_BL_RotateBltColorFill16

#define BL_RotateBltKeySrcColorFill555		_BL_RotateBltKeySrcColorFill16

/**************************************************************
* RGB565 RotateBlt Functions
**************************************************************/

#define BL_RotateBlt565				_BL_RotateBlt16
	
#define BL_RotateBltKeySrc565		_BL_RotateBltKeySrc16
	
#define BL_RotateBltColorFill565		_BL_RotateBltColorFill16

#define BL_RotateBltKeySrcColorFill565		_BL_RotateBltKeySrcColorFill16

/**************************************************************
* Generic 16-bit RotateBlt Functions
**************************************************************/

void _BL_RotateBlt16(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src,
	LONG srcXPitch, LONG srcYPitch, LONG srcRectWidth, LONG srcRectHeight);

void _BL_RotateBltKeySrc16(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcRectWidth, LONG srcRectHeight, DWORD dwNativeColorKey);

void _BL_RotateBltColorFill16(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, LONG srcRectWidth,
	LONG srcRectHeight, DWORD dwNativeColorFill);
	
void _BL_RotateBltKeySrcColorFill16(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcRectWidth, LONG srcRectHeight, DWORD dwNativeColorKey, DWORD dwNativeColorFill);

#ifdef __cplusplus
}
#endif

#endif // __BL_STRETCHBLT_H_