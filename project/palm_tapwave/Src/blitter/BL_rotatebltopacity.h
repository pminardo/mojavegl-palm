/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_ROTATEBLTOPACITY_H_
#define __BL_ROTATEBLTOPACITY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 RotateBltOpacity Functions
**************************************************************/

#define BL_RotateBltOpacity555\
	#error "Not yet implemented"
	
#define BL_RotateBltOpacityKeySrc555\
	#error "Not yet implemented"
	
#define BL_RotateBltOpacityColorFill555\
	#error "Not yet implemented"

#define BL_RotateBltOpacityKeySrcColorFill555\
	#error "Not yet implemented"

/**************************************************************
* RGB565 RotateBltOpacity Functions
**************************************************************/

void BL_RotateBltOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src,
	LONG srcXPitch, LONG srcYPitch, LONG srcRectWidth, LONG srcRectHeight,
	DWORD dwOpacity);

void BL_RotateBltOpacityKeySrc565(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcRectWidth, LONG srcRectHeight, DWORD dwNativeColorKey,
	DWORD dwOpacity);

void BL_RotateBltOpacityColorFill565(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, LONG srcRectWidth,
	LONG srcRectHeight, DWORD dwNativeColorFill, DWORD dwOpacity);
	
void BL_RotateBltOpacityKeySrcColorFill565(void *dest, LONG destXPitch, LONG destYPitch,
	const RECT *pDestRect, DWORD *tmatrix, void *src, LONG srcXPitch, LONG srcYPitch,
	LONG srcRectWidth, LONG srcRectHeight, DWORD dwNativeColorKey, DWORD dwNativeColorFill,
	DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_ROTATEBLTOPACITY_H_