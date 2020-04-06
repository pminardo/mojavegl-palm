/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_ALPHABLTSUBTRACTIVE_H_
#define __BL_ALPHABLTSUBTRACTIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 AlphaBltSubtractive Functions
**************************************************************/

#define BL_AlphaBltSubtractive555\
	#error "Not yet supported"

#define BL_AlphaBltSubtractiveOpacity555\
	#error "Not yet supported"

/**************************************************************
* RGB565 AlphaBltSubtractive Functions
**************************************************************/

void BL_AlphaBltSubtractive565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG srcXPitch, LONG srcYPitch,
	LONG alphaXPitch, LONG alphaYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, LONG alphaX, LONG alphaY,
	LONG alphaWidth, LONG alphaHeight);

void BL_AlphaBltSubtractiveOpacity565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG srcXPitch, LONG srcYPitch,
	LONG alphaXPitch, LONG alphaYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, LONG alphaX, LONG alphaY,
	LONG alphaWidth, LONG alphaHeight, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_ALPHABLTSUBTRACTIVE_H_