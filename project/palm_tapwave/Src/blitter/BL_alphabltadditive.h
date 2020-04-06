/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_ALPHABLTADDITIVE_H_
#define __BL_ALPHABLTADDITIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* RGB555 AlphaBltAdditive Functions
**************************************************************/

#define BL_AlphaBltAdditive555\
	#error "Not yet supported"

#define BL_AlphaBltAdditiveOpacity555\
	#error "Not yet supported"

/**************************************************************
* RGB565 AlphaBltAdditive Functions
**************************************************************/

void BL_AlphaBltAdditive565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG srcXPitch, LONG srcYPitch,
	LONG alphaXPitch, LONG alphaYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, LONG alphaX, LONG alphaY,
	LONG alphaWidth, LONG alphaHeight);

void BL_AlphaBltAdditiveOpacity565(void *dest, const void *src, const void *alpha,
	LONG destXPitch, LONG destYPitch, LONG srcXPitch, LONG srcYPitch,
	LONG alphaXPitch, LONG alphaYPitch, LONG destX, LONG destY,
	LONG destWidth, LONG destHeight, LONG srcX, LONG srcY,
	LONG srcWidth, LONG srcHeight, LONG alphaX, LONG alphaY,
	LONG alphaWidth, LONG alphaHeight, DWORD dwOpacity);

#ifdef __cplusplus
}
#endif

#endif // __BL_ALPHABLTADDITIVE_H_