/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_FILLGRADIENT_TWGFX_H_
#define __BL_FILLGRADIENT_TWGFX_H_

#ifdef __cplusplus
extern "C" {
#endif

// Performs a gradient fill; uses TwGfx video hw.
// IMPORTANT NOTE: This function does not support opacity!
void BL_FillGradient_TwGfx(TwGfxSurfaceHandle twDestSurface,
	LONG dx, LONG dy, LONG dx2, LONG dy2, COLORREF dwColor1,
	COLORREF dwColor2, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif // __BL_FILLGRADIENT_TWGFX_H_