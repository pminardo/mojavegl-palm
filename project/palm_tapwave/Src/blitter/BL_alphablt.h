/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_ALPHABLT_H_
#define __BL_ALPHABLT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 AlphaBlt Functions
**************************************************************/

#define BL_AlphaBlt555\
	#error "Not yet supported"

#define BL_AlphaBltOpacity555\
	#error "Not yet supported"

/**************************************************************
* RGB565 AlphaBlt Functions
**************************************************************/

void BL_AlphaBlt565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG srcXPitch, LONG srcYPitch,
	LONG alphaXPitch, LONG alphaYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, LONG alphaX, LONG alphaY,
	LONG alphaWidth, LONG alphaHeight);

void BL_AlphaBltOpacity565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG srcXPitch, LONG srcYPitch,
	LONG alphaXPitch, LONG alphaYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, LONG alphaX, LONG alphaY,
	LONG alphaWidth, LONG alphaHeight, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_ALPHABLT_H_