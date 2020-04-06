/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGLDefines.h"
#include "BL_pixelutil.h"
#include "BL_drawline.h"

/**************************************************************
* Helper Functions
**************************************************************/

namespace DrawWuLine565 {

	inline
	DWORD BlendPixelAverage(DWORD pixel, DWORD backpixel, DWORD opacity, DWORD masterOpacity)
	{
		// Compute the average of the opacity values
		opacity = pxutil::AverageAlpha(opacity, masterOpacity);
	
		// Perform the AlphaBlend
		return pxutil565::AlphaBlend(pixel, backpixel, opacity);
	}

};

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ RGB565 DrawLine Functions --------------
#endif

// Performs a 16-bit DrawVLine() operation.
void BL_DrawVLine565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x, LONG y1, LONG y2,
	COLORREF dwColor)
{
	DWORD dwNativeColor;
	WORD *pDest;
	
	// Handle clipping, if needed
	if (pClipper && !IsRectEmpty(pClipper))
	{
		if ((x < pClipper->left) || (x >= pClipper->right)) return;
		if (y1 < pClipper->top) y1 = pClipper->top;
		if (y2 >= pClipper->bottom) y2 = pClipper->bottom-1;
		if (y2 < y1) return;
	}
	
	// Swap coordinates, if needed
	if (y2 < y1)
	{
		LONG temp = y2;
		y2 = y1;
		y1 = temp;
	}

	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	
	// Get a ptr to the destination
	pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x, y1);

	for(LONG y = y1 ; y <= y2; y++)
	{
		*pDest = (WORD)dwNativeColor;
		
		// Advance to next row
		pDest += destYPitch;
	}
}

// Performs a 16-bit DrawHLine() operation.
void BL_DrawHLine565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x1,
	LONG y, LONG x2, COLORREF dwColor)
{
	DWORD dwNativeColor;
	WORD *pDest;
	
	// Handle clipping, if needed
	if (pClipper && !IsRectEmpty(pClipper))
	{
		if ((y < pClipper->top) || (y >= pClipper->bottom)) return;
		if (x1 < pClipper->left) x1 = pClipper->left;
		if (x2 >= pClipper->right) x2 = pClipper->right-1;
		if (x2 < x1) return;
	}
	
	// Swap coordinates, if needed
	if (x2 < x1)
	{
		LONG temp = x2;
		x2 = x1;
		x1 = temp;
	}

	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	
	// Get a ptr to the destination
	pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1, y);

	for (LONG x = x1; x <= x2; x++)
	{
		*pDest = (WORD)dwNativeColor;
		
		// Advance to the next pixel
		pDest += destXPitch;
	}
}

// Performs a 16-bit DrawVLine() operation with variable opacity.
void BL_DrawVLineOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x, LONG y1, LONG y2,
	COLORREF dwColor, DWORD dwOpacity)
{
	DWORD dwNativeColor;
	WORD *pDest;
	
	// Handle clipping, if needed
	if (pClipper && !IsRectEmpty(pClipper))
	{
		if ((x < pClipper->left) || (x >= pClipper->right)) return;
		if (y1 < pClipper->top) y1 = pClipper->top;
		if (y2 >= pClipper->bottom) y2 = pClipper->bottom-1;
		if (y2 < y1) return;
	}
	
	// Swap coordinates, if needed
	if (y2 < y1)
	{
		LONG temp = y2;
		y2 = y1;
		y1 = temp;
	}

	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	
	// Get a ptr to the destination
	pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x, y1);
	
	// Optimize for 50% opacity
	if (dwOpacity == OPACITY_50)
	{
		for(LONG y = y1 ; y <= y2; y++)
		{
			*pDest = (WORD)pxutil565::AlphaBlendFast(dwNativeColor, *pDest);
			
			// Advance to next row
			pDest += destYPitch;
		}
	}
	else
	{
		for(LONG y = y1 ; y <= y2; y++)
		{
			*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwOpacity);
			
			// Advance to next row
			pDest += destYPitch;
		}
	}
}

// Performs a 16-bit DrawHLine() operation with variable opacity.
void BL_DrawHLineOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG x1,
	LONG y, LONG x2, COLORREF dwColor, DWORD dwOpacity)
{
	DWORD dwNativeColor;
	WORD *pDest;
	
	// Handle clipping, if needed
	if (pClipper && !IsRectEmpty(pClipper))
	{
		if ((y < pClipper->top) || (y >= pClipper->bottom)) return;
		if (x1 < pClipper->left) x1 = pClipper->left;
		if (x2 >= pClipper->right) x2 = pClipper->right-1;
		if (x2 < x1) return;
	}
	
	// Swap coordinates, if needed
	if (x2 < x1)
	{
		LONG temp = x2;
		x2 = x1;
		x1 = temp;
	}

	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	
	// Get a ptr to the destination
	pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1, y);
	
	// Optimize for 50% opacity
	if (dwOpacity == OPACITY_50)
	{
		for (LONG x = x1; x <= x2; x++)
		{
			*pDest = (WORD)pxutil565::AlphaBlendFast(dwNativeColor, *pDest);

			// Advance to the next pixel
			pDest += destXPitch;
		}
	}
	else
	{
		for (LONG x = x1; x <= x2; x++)
		{
			*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwOpacity);

			// Advance to the next pixel
			pDest += destXPitch;
		}
	}
}

// Performs a 16-bit DrawLine() operation.
void BL_DrawLine565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG x1, LONG y1, LONG x2, LONG y2,
	COLORREF dwColor)
{
	// Draw a vertical line
	if (x1 == x2)
	{
		BL_DrawVLine565(dest, destXPitch, destYPitch,
			destWidth, destHeight, NULL, x1, y1, y2, dwColor);
	}
	// Draw a horizontal line
	else if (y1 == y2)
	{
		BL_DrawHLine565(dest, destXPitch, destYPitch,
			destWidth, destHeight, NULL, x1, y1, x2, dwColor);
	}
	else
	{
		LONG dx,		//deltas
			dy,
			dx2,		//scaled deltas
			dy2,
			ix,		//increase rate on the x axis
			iy,		//increase rate on the y axis
			err,		//the error term
			i;		//looping variable

		DWORD dwNativeColor = pxutil565::ColorrefToNative(dwColor);
		
		// identify the first pixel
		WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1, y1);
	
		// difference between starting and ending points
		dx = x2 - x1;
		dy = y2 - y1;

		// calculate direction of the vector and store in ix and iy
		if (dx >= 0)
			ix = destXPitch;

		if (dx < 0)
		{
			ix = -destXPitch;
			dx = abs(dx);
		}

		if (dy >= 0)
			iy = destYPitch;

		if (dy < 0)
		{
			iy = -destYPitch;
			dy = abs(dy);
		}

		// scale deltas and store in dx2 and dy2
		dx2 = dx * 2;
		dy2 = dy * 2;
		
		if (dx > dy)	// dx is the major axis
		{
			// initialize the error term
			err = dy2 - dx;

			for (i = 0; i <= dx; i++)
			{
				*pDest = (WORD)dwNativeColor;
				if (err >= 0)
				{
					err -= dx2;
					pDest += iy;
				}
				err += dy2;
				pDest += ix;
			}
		}
		
		else 		// dy is the major axis
		{
			// initialize the error term
			err = dx2 - dy;

			for (i = 0; i <= dy; i++)
			{
				*pDest = (WORD)dwNativeColor;
				if (err >= 0)
				{
					err -= dy2;
					pDest += ix;
				}
				err += dx2;
				pDest += iy;
			}
		}
	}
}

// Performs a 16-bit DrawLine() operation with variable opacity.
void BL_DrawLineOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG x1, LONG y1, LONG x2, LONG y2,
	COLORREF dwColor, DWORD dwOpacity)
{
	// Draw a vertical line
	if (x1 == x2)
	{
		BL_DrawVLineOpacity565(dest, destXPitch, destYPitch,
			destWidth, destHeight, NULL, x1, y1, y2, dwColor, dwOpacity);
	}
	// Draw a horizontal line
	else if (y1 == y2)
	{
		BL_DrawHLineOpacity565(dest, destXPitch, destYPitch,
			destWidth, destHeight, NULL, x1, y1, x2, dwColor, dwOpacity);
	}
	else
	{
		LONG dx,		//deltas
			dy,
			dx2,		//scaled deltas
			dy2,
			ix,		//increase rate on the x axis
			iy,		//increase rate on the y axis
			err,		//the error term
			i;		//looping variable

		DWORD dwNativeColor = pxutil565::ColorrefToNative(dwColor);
		
		// identify the first pixel
		WORD *pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, x1, y1);
	
		// difference between starting and ending points
		dx = x2 - x1;
		dy = y2 - y1;

		// calculate direction of the vector and store in ix and iy
		if (dx >= 0)
			ix = destXPitch;

		if (dx < 0)
		{
			ix = -destXPitch;
			dx = abs(dx);
		}

		if (dy >= 0)
			iy = destYPitch;

		if (dy < 0)
		{
			iy = -destYPitch;
			dy = abs(dy);
		}

		// scale deltas and store in dx2 and dy2
		dx2 = dx * 2;
		dy2 = dy * 2;
		
		if (dx > dy)	// dx is the major axis
		{
			// initialize the error term
			err = dy2 - dx;

			for (i = 0; i <= dx; i++)
			{
				*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwOpacity);
				if (err >= 0)
				{
					err -= dx2;
					pDest += iy;
				}
				err += dy2;
				pDest += ix;
			}
		}
		
		else 		// dy is the major axis
		{
			// initialize the error term
			err = dx2 - dy;

			for (i = 0; i <= dy; i++)
			{
				*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwOpacity);
				if (err >= 0)
				{
					err -= dy2;
					pDest += ix;
				}
				err += dx2;
				pDest += iy;
			}
		}
	}
}

// Performs a 16-bit DrawLine() operation with antialiasing.
void BL_DrawWuLine565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG X0, LONG Y0, LONG X1, LONG Y1,
	COLORREF dwColor)
{
	DWORD dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	WORD *pDest;
	DWORD dwOpacity;

	/* Make sure the line runs top to bottom */
	if (Y0 > Y1)
	{
	    LONG Temp = Y0; Y0 = Y1; Y1 = Temp;
	    Temp = X0; X0 = X1; X1 = Temp;
	}
    
	/* Draw the initial pixel, which is always exactly intersected by
	the line and so needs no weighting */
	
	// Plot pixel
	pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
	*pDest = (WORD)dwNativeColor;
	
	LONG XDir, DeltaX = X1 - X0;
	if( DeltaX >= 0 )
	{
		XDir = 1;
	}
	else
	{
		XDir   = -1;
		DeltaX = 0 - DeltaX; /* make DeltaX positive */
	}
    
	/* Special-case horizontal, vertical, and diagonal lines, which
	require no weighting because they go right through the center of
	every pixel */
	LONG DeltaY = Y1 - Y0;
	if (DeltaY == 0)
	{
		/* Horizontal line */
		while (DeltaX-- != 0)
		{
			X0 += XDir;

			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
			*pDest = (WORD)dwNativeColor;
		}
		
		return;
	}
    
	if (DeltaX == 0)
	{
		/* Vertical line */
		do
		{
			Y0++;
			
			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
			*pDest = (WORD)dwNativeColor;

		} while (--DeltaY != 0);
		
		return;
	}
    
	if (DeltaX == DeltaY)
	{
		/* Diagonal line */
		do
		{
			X0 += XDir;
			Y0++;
			
			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
			*pDest = (WORD)dwNativeColor;

		} while (--DeltaY != 0);
		
		return;
	}
    
	WORD ErrorAdj;
	WORD ErrorAccTemp, Weighting;

	/* Line is not horizontal, diagonal, or vertical */
	WORD ErrorAcc = 0;  /* initialize the line error accumulator to 0 */

	/* Is this an X-major or Y-major line? */
	if (DeltaY > DeltaX)
	{
		/* Y-major line; calculate 16-bit fixed-point fractional part of a
		pixel that X advances each time Y advances 1 pixel, truncating the
		result so that we won't overrun the endpoint along the X axis */
		ErrorAdj = (WORD)(((DWORD) DeltaX << 16) / (DWORD) DeltaY);

		/* Draw all pixels other than the first and last */
		while (--DeltaY)
		{
			ErrorAccTemp = ErrorAcc;   	/* remember currrent accumulated error */
			ErrorAcc += ErrorAdj;      		/* calculate error for next pixel */
			
			if (ErrorAcc <= ErrorAccTemp) {
				/* The error accumulator turned over, so advance the X coord */
				X0 += XDir;
			}
			
			Y0++; /* Y-major, so always advance Y */
			/* The IntensityBits most significant bits of ErrorAcc give us the
			intensity weighting for this pixel, and the complement of the
			weighting for the paired pixel */
			Weighting = ErrorAcc >> 8;
			
			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
			dwOpacity = (BYTE)(Weighting ^ 255);
			*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwOpacity);

			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0 + XDir, Y0);
			dwOpacity = (BYTE)Weighting;
			*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwOpacity);
		}
		
		/* Draw the final pixel, which is always exactly intersected by the line
		and so needs no weighting */
		
		// Plot pixel
		pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X1, Y1);
		*pDest = (WORD)dwNativeColor;
		
		return;
	}

	/* It's an X-major line; calculate 16-bit fixed-point fractional part of a
	pixel that Y advances each time X advances 1 pixel, truncating the
	result to avoid overrunning the endpoint along the X axis */
	ErrorAdj = (WORD)(((DWORD) DeltaY << 16) / (DWORD) DeltaX);
	
	/* Draw all pixels other than the first and last */
	while (--DeltaX) {
		ErrorAccTemp = ErrorAcc;   	/* remember currrent accumulated error */
		ErrorAcc += ErrorAdj;      		/* calculate error for next pixel */
		
		if (ErrorAcc <= ErrorAccTemp) {
			/* The error accumulator turned over, so advance the Y coord */
			Y0++;
		}
		
		X0 += XDir; /* X-major, so always advance X */
		/* The IntensityBits most significant bits of ErrorAcc give us the
		intensity weighting for this pixel, and the complement of the
		weighting for the paired pixel */
		Weighting = ErrorAcc >> 8;
		
		// Plot pixel
		pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
		dwOpacity = (BYTE)(Weighting ^ 255);
		*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwOpacity);

		// Plot pixel
		pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0 + 1);
		dwOpacity = (BYTE)Weighting;
		*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwOpacity);
	}
    
	/* Draw the final pixel, which is always exactly intersected by the line
	and so needs no weighting */
	
	// Plot pixel
	pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X1, Y1);
	*pDest = (WORD)dwNativeColor;
}

// Performs a 16-bit DrawLine() operation with antialiasing and variable opacity.
void BL_DrawWuLineOpacity565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, LONG X0, LONG Y0, LONG X1, LONG Y1,
	COLORREF dwColor, DWORD dwMasterOpacity)
{
	DWORD dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	WORD *pDest;
	DWORD dwOpacity;

	/* Make sure the line runs top to bottom */
	if (Y0 > Y1)
	{
	    LONG Temp = Y0; Y0 = Y1; Y1 = Temp;
	    Temp = X0; X0 = X1; X1 = Temp;
	}
    
	/* Draw the initial pixel, which is always exactly intersected by
	the line and so needs no weighting */
	
	// Plot pixel
	pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
	*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwMasterOpacity);
	
	LONG XDir, DeltaX = X1 - X0;
	if( DeltaX >= 0 )
	{
		XDir = 1;
	}
	else
	{
		XDir   = -1;
		DeltaX = 0 - DeltaX; /* make DeltaX positive */
	}
    
	/* Special-case horizontal, vertical, and diagonal lines, which
	require no weighting because they go right through the center of
	every pixel */
	LONG DeltaY = Y1 - Y0;
	if (DeltaY == 0)
	{
		/* Horizontal line */
		while (DeltaX-- != 0)
		{
			X0 += XDir;

			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
			*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwMasterOpacity);
		}
		
		return;
	}
    
	if (DeltaX == 0)
	{
		/* Vertical line */
		do
		{
			Y0++;
			
			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
			*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwMasterOpacity);

		} while (--DeltaY != 0);
		
		return;
	}
    
	if (DeltaX == DeltaY)
	{
		/* Diagonal line */
		do
		{
			X0 += XDir;
			Y0++;
			
			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
			*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwMasterOpacity);

		} while (--DeltaY != 0);
		
		return;
	}
    
	WORD ErrorAdj;
	WORD ErrorAccTemp, Weighting;

	/* Line is not horizontal, diagonal, or vertical */
	WORD ErrorAcc = 0;  /* initialize the line error accumulator to 0 */

	/* Is this an X-major or Y-major line? */
	if (DeltaY > DeltaX)
	{
		/* Y-major line; calculate 16-bit fixed-point fractional part of a
		pixel that X advances each time Y advances 1 pixel, truncating the
		result so that we won't overrun the endpoint along the X axis */
		ErrorAdj = (WORD)(((DWORD) DeltaX << 16) / (DWORD) DeltaY);

		/* Draw all pixels other than the first and last */
		while (--DeltaY)
		{
			ErrorAccTemp = ErrorAcc;   	/* remember currrent accumulated error */
			ErrorAcc += ErrorAdj;      		/* calculate error for next pixel */
			
			if (ErrorAcc <= ErrorAccTemp) {
				/* The error accumulator turned over, so advance the X coord */
				X0 += XDir;
			}
			
			Y0++; /* Y-major, so always advance Y */
			/* The IntensityBits most significant bits of ErrorAcc give us the
			intensity weighting for this pixel, and the complement of the
			weighting for the paired pixel */
			Weighting = ErrorAcc >> 8;
			
			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
			dwOpacity = (BYTE)(Weighting ^ 255);
			*pDest = (WORD)DrawWuLine565::BlendPixelAverage(dwNativeColor, *pDest, dwOpacity, dwMasterOpacity);

			// Plot pixel
			pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0 + XDir, Y0);
			dwOpacity = (BYTE)Weighting;
			*pDest = (WORD)DrawWuLine565::BlendPixelAverage(dwNativeColor, *pDest, dwOpacity, dwMasterOpacity);
		}
		
		/* Draw the final pixel, which is always exactly intersected by the line
		and so needs no weighting */
		
		// Plot pixel
		pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X1, Y1);
		*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwMasterOpacity);
		
		return;
	}

	/* It's an X-major line; calculate 16-bit fixed-point fractional part of a
	pixel that Y advances each time X advances 1 pixel, truncating the
	result to avoid overrunning the endpoint along the X axis */
	ErrorAdj = (WORD)(((DWORD) DeltaY << 16) / (DWORD) DeltaX);
	
	/* Draw all pixels other than the first and last */
	while (--DeltaX) {
		ErrorAccTemp = ErrorAcc;   	/* remember currrent accumulated error */
		ErrorAcc += ErrorAdj;      		/* calculate error for next pixel */
		
		if (ErrorAcc <= ErrorAccTemp) {
			/* The error accumulator turned over, so advance the Y coord */
			Y0++;
		}
		
		X0 += XDir; /* X-major, so always advance X */
		/* The IntensityBits most significant bits of ErrorAcc give us the
		intensity weighting for this pixel, and the complement of the
		weighting for the paired pixel */
		Weighting = ErrorAcc >> 8;
		
		// Plot pixel
		pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0);
		dwOpacity = (BYTE)(Weighting ^ 255);
		*pDest = (WORD)DrawWuLine565::BlendPixelAverage(dwNativeColor, *pDest, dwOpacity, dwMasterOpacity);

		// Plot pixel
		pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X0, Y0 + 1);
		dwOpacity = (BYTE)Weighting;
		*pDest = (WORD)DrawWuLine565::BlendPixelAverage(dwNativeColor, *pDest, dwOpacity, dwMasterOpacity);
	}
    
	/* Draw the final pixel, which is always exactly intersected by the line
	and so needs no weighting */
	
	// Plot pixel
	pDest = pxutil16bit::GetBuffPixels(dest, destXPitch, destYPitch, X1, Y1);
	*pDest = (WORD)pxutil565::AlphaBlend(dwNativeColor, *pDest, dwMasterOpacity);
}