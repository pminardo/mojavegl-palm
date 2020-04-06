/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_pixelutil.h"
#include "BL_fillrect.h"
#include "BL_fillgradient.h"

/**************************************************************
* Dithering Tables
**************************************************************/

static const DWORD dither_table[8] = { 0, 16, 68, 146, 170, 109, 187, 239 };
static const DWORD dither_ytable[8] = { 1, 5, 2, 7, 4, 0, 6, 3 };

/**************************************************************
* Internal Functions
**************************************************************/

namespace FillGradientDither555 {
	
	// Calculates a dithered RGB555 pixel value.
	inline
	DWORD MakeColor(DWORD r, DWORD g, DWORD b, LONG x, LONG y)
	{
		DWORD returned_r, returned_g, returned_b;
		DWORD bpos;

		returned_r = r>>3;
		returned_g = g>>3;
		returned_b = b>>3;

		y = dither_ytable[y&7];

		bpos = (x+y)&7; 
		returned_r += (dither_table[r&7] >> bpos) & 1;

		bpos = (bpos+3)&7;
		returned_b += (dither_table[b&7] >> bpos) & 1;

		bpos = (bpos+7)&7;
		returned_g += (dither_table[g&7] >> bpos) & 1;

		returned_r -= returned_r>>5;
		returned_g -= returned_g>>5;
		returned_b -= returned_b>>5;

		return (returned_r << 10) | (returned_g << 5) | (returned_b /*<< 0*/);

	}

	// Performs a dithered FillRect() operation.
	inline
	void FillRect(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, LONG x, LONG y,
		LONG rw, LONG rh, COLORREF dwColor)
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD dwDestOffset;
		DWORD r, g, b;
		
		// Extract color components
		r = GetRValue(dwColor);
		g = GetGValue(dwColor);
		b = GetBValue(dwColor);
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute dest offset
		dwDestOffset = destYPitch - rw;
		
		// Init dest pointer
		pDest16 += x + (y * destYPitch);
		
		while (rh != 0) {
		
			#define BLIT_EXPR\
			{\
				*pDest16++ = (WORD)MakeColor(r, g, b, x, y);\
				++x;\
			}
			
			BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			--rh;
			++y;
		}
	}
	
	// Performs a dithered FillRectOpacity() operation.
	inline
	void FillRectOpacity(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, LONG x, LONG y,
		LONG rw, LONG rh, COLORREF dwColor, DWORD dwOpacity)
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD dwDestOffset;
		DWORD r, g, b;
		
		// Extract color components
		r = GetRValue(dwColor);
		g = GetGValue(dwColor);
		b = GetBValue(dwColor);
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute dest offset
		dwDestOffset = destYPitch - rw;
		
		// Init dest pointer
		pDest16 += x + (y * destYPitch);
		
		// Optimize for 50% fast blending
		if (dwOpacity == OPACITY_50)
		{
			while (rh != 0) {
			
				#define BLIT_EXPR\
				{\
					*pDest16++ = (WORD)pxutil555::AlphaBlendFast(MakeColor(r, g, b, x, y), *pDest16);\
					++x;\
				}
				
				BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
				
				#undef BLIT_EXPR
			
				pDest16 += dwDestOffset;
				--rh;
				++y;
			}
		}
		else
		{
			while (rh != 0) {
			
				#define BLIT_EXPR\
				{\
					*pDest16++ = (WORD)pxutil555::AlphaBlend(MakeColor(r, g, b, x, y), *pDest16, dwOpacity);\
					++x;\
				}
				
				BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
				
				#undef BLIT_EXPR
			
				pDest16 += dwDestOffset;
				--rh;
				++y;
			}
		}
	}

}

namespace FillGradientDither565 {
	
	// Calculates a dithered RGB565 pixel value.
	inline
	DWORD MakeColor(DWORD r, DWORD g, DWORD b, LONG x, LONG y)
	{
	   DWORD returned_r, returned_g, returned_b;
	   DWORD bpos;

	   returned_r = r>>3;
	   returned_g = g>>2;
	   returned_b = b>>3;

	   y = dither_ytable[y&7];

	   bpos = (x+y)&7; 
	   returned_r += (dither_table[r&7] >> bpos) & 1;
	   
	   bpos = (bpos+3)&7;
	   returned_b += (dither_table[b&7] >> bpos) & 1;

	   bpos = (bpos+7)&7;
	   returned_g += (dither_table[(g&3)/**2*/<<1] >> bpos) & 1;

	   returned_r -= returned_r>>5;
	   returned_g -= returned_g>>6;
	   returned_b -= returned_b>>5;

	   return (returned_r << 11) | (returned_g << 5) | (returned_b /*<< 0*/);
	}

	// Performs a dithered FillRect() operation.
	inline
	void FillRect(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, LONG x, LONG y,
		LONG rw, LONG rh, COLORREF dwColor)
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD dwDestOffset;
		DWORD r, g, b;
		
		// Extract color components
		r = GetRValue(dwColor);
		g = GetGValue(dwColor);
		b = GetBValue(dwColor);
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute dest offset
		dwDestOffset = destYPitch - rw;
		
		// Init dest pointer
		pDest16 += x + (y * destYPitch);
		
		while (rh != 0) {
		
			#define BLIT_EXPR\
			{\
				*pDest16++ = (WORD)MakeColor(r, g, b, x, y);\
				++x;\
			}
			
			BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
			
			#undef BLIT_EXPR
		
			pDest16 += dwDestOffset;
			--rh;
			++y;
		}
	}

	// Performs a dithered FillRectOpacity() operation.
	inline
	void FillRectOpacity(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, LONG x, LONG y,
		LONG rw, LONG rh, COLORREF dwColor, DWORD dwOpacity)
	{
		WORD *pDest16 = (WORD*)dest;
		DWORD dwDestOffset;
		DWORD r, g, b;
		
		// Extract color components
		r = GetRValue(dwColor);
		g = GetGValue(dwColor);
		b = GetBValue(dwColor);
		
		// Optimize destination pitch
		pxutil::OptimizePitch(pDest16, destWidth, destHeight, destXPitch, destYPitch);
		
		// Compute dest offset
		dwDestOffset = destYPitch - rw;
		
		// Init dest pointer
		pDest16 += x + (y * destYPitch);
		
		// Optimize for 50% fast blending
		if (dwOpacity == OPACITY_50)
		{
			while (rh != 0) {
			
				#define BLIT_EXPR\
				{\
					*pDest16++ = (WORD)pxutil565::AlphaBlendFast(MakeColor(r, g, b, x, y), *pDest16);\
					++x;\
				}
				
				BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
				
				#undef BLIT_EXPR
			
				pDest16 += dwDestOffset;
				--rh;
				++y;
			}
		}
		else
		{
			while (rh != 0) {
			
				#define BLIT_EXPR\
				{\
					*pDest16++ = (WORD)pxutil565::AlphaBlend(MakeColor(r, g, b, x, y), *pDest16, dwOpacity);\
					++x;\
				}
				
				BLITTER_DUFF_DEVICE(rw, BLIT_EXPR);
				
				#undef BLIT_EXPR
			
				pDest16 += dwDestOffset;
				--rh;
				++y;
			}
		}
	}
}

/**************************************************************
* Internal Functions
**************************************************************/

void BL_FillGradient565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG dx, LONG dy,
	LONG dx2, LONG dy2, COLORREF dwColor1, COLORREF dwColor2,
	DWORD dwFlags, DWORD dwOpacity)
{
	DOUBLE rStep, gStep, bStep;
	DOUBLE rCount, gCount, bCount;
	const LONG NUM_COLORS = 256;
	LONG ColorCount;
	DWORD dwNativeColor;
	RECT rect;
	BOOL bClippingEnabled;
	
	// Determine whether or not clipping is enabled
	bClippingEnabled = !IsRectEmpty(pClipper);
	
	if (dwFlags & MGLFILLGRADIENT_HORIZ)
	{
		DOUBLE RectWidth = (dx2 - dx) / 256.0;
		
		//rStep,gStep, and bStep are variables that will be used
		//to hold the values at which R,G,B will be changed respectivily
		//For example: if we have RTop=100 and RBot=150 then 
		//rStep=(150-100)/256 so when we start at R=100 and draw 256 rectangles
		//we will end at R=150 when we finish drawing these rectangles 

		//We will start counting from TopColor to BottomColor
		//So we give an initial value of the TopColor
		rCount = (DOUBLE)GetRValue(dwColor1);
		gCount = (DOUBLE)GetGValue(dwColor1);
		bCount = (DOUBLE)GetBValue(dwColor1);
		
		//Calcualte the step of R,G,B values
		rStep = -((DOUBLE)rCount-GetRValue(dwColor2))/NUM_COLORS;
		gStep = -((DOUBLE)gCount-GetGValue(dwColor2))/NUM_COLORS;
		bStep = -((DOUBLE)bCount-GetBValue(dwColor2))/NUM_COLORS;
		
		for (ColorCount=0; ColorCount < NUM_COLORS; ++ColorCount)
		{
			//Draw using current RGB values and Change RGB values
			//to represent the next color in the chain
			dwNativeColor = pxutil565::RGBToNative((DWORD)rCount, (DWORD)gCount, (DWORD)bCount);

			rect.top = dy;
			rect.bottom = dy2;
			rect.left = (LONG)(dx+(ColorCount*RectWidth));
			rect.right = (LONG)(dx+((ColorCount+1)*RectWidth));
			
			// Perform clipping, if needed
			if (bClippingEnabled)
			{
				if (!IntersectRect(&rect, &rect, pClipper))
				{
					rCount += rStep;
					gCount += gStep;
					bCount += bStep;
					
					continue;
				}
			}
			
			// Perform the FillRect()
			if (dwFlags & MGLFILLGRADIENT_OPACITY)
			{
				BL_FillRectOpacity565(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					rect.left,
					rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					dwNativeColor,
					dwOpacity);
			}
			else
			{
				BL_FillRect565(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					rect.left,
					rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					dwNativeColor);
			}

			rCount += rStep;
			gCount += gStep;
			bCount += bStep;
		}
	}
	else //if (dwFlags & MGLFILLRECTGRADIENT_VERT)
	{
		DOUBLE RectHeight = (dy2 - dy) / 256.0;
		
		//rStep,gStep, and bStep are variables that will be used
		//to hold the values at which R,G,B will be changed respectivily
		//For example: if we have RTop=100 and RBot=150 then 
		//rStep=(150-100)/256 so when we start at R=100 and draw 256 rectangles
		//we will end at R=150 when we finish drawing these rectangles 

		//We will start counting from TopColor to BottomColor
		//So we give an initial value of the TopColor
		rCount = (DOUBLE)GetRValue(dwColor1);
		gCount = (DOUBLE)GetGValue(dwColor1);
		bCount = (DOUBLE)GetBValue(dwColor1);
		
		//Calcualte the step of R,G,B values
		rStep = -((DOUBLE)rCount-GetRValue(dwColor2))/NUM_COLORS;
		gStep = -((DOUBLE)gCount-GetGValue(dwColor2))/NUM_COLORS;
		bStep = -((DOUBLE)bCount-GetBValue(dwColor2))/NUM_COLORS;
		
		for (ColorCount=0; ColorCount < NUM_COLORS; ++ColorCount)
		{
			//Draw using current RGB values and Change RGB values
			//to represent the next color in the chain
			dwNativeColor = pxutil565::RGBToNative((DWORD)rCount, (DWORD)gCount, (DWORD)bCount);

			rect.left = dx;
			rect.right = dx2;
			rect.top = (LONG)(dy+(ColorCount*RectHeight));
			rect.bottom = (LONG)(dy+((ColorCount+1)*RectHeight));
			
			// Perform clipping, if needed
			if (bClippingEnabled)
			{
				if (!IntersectRect(&rect, &rect, pClipper))
				{
					rCount += rStep;
					gCount += gStep;
					bCount += bStep;
					
					continue;
				}
			}
			
			// Perform the FillRect()
			if (dwFlags & MGLFILLGRADIENT_OPACITY)
			{
				BL_FillRectOpacity565(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					rect.left,
					rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					dwNativeColor,
					dwOpacity);
			}
			else
			{
				BL_FillRect565(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					rect.left,
					rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					dwNativeColor);
			}
			
			rCount += rStep;
			gCount += gStep;
			bCount += bStep;
		}
	}
}

void BL_FillGradientDither565(void *dest, LONG destXPitch, LONG destYPitch,
	LONG destWidth, LONG destHeight, RECT *pClipper, LONG dx, LONG dy,
	LONG dx2, LONG dy2, COLORREF dwColor1, COLORREF dwColor2,
	DWORD dwFlags, DWORD dwOpacity)
{
	DOUBLE rStep, gStep, bStep;
	DOUBLE rCount, gCount, bCount;
	const LONG NUM_COLORS = 256;
	LONG ColorCount;
	DWORD dwColor;
	RECT rect;
	BOOL bClippingEnabled;
	
	// Determine whether or not clipping is enabled
	bClippingEnabled = !IsRectEmpty(pClipper);
	
	if (dwFlags & MGLFILLGRADIENT_HORIZ)
	{
		DOUBLE RectWidth = (dx2 - dx) / 256.0;
		
		//rStep,gStep, and bStep are variables that will be used
		//to hold the values at which R,G,B will be changed respectivily
		//For example: if we have RTop=100 and RBot=150 then 
		//rStep=(150-100)/256 so when we start at R=100 and draw 256 rectangles
		//we will end at R=150 when we finish drawing these rectangles 

		//We will start counting from TopColor to BottomColor
		//So we give an initial value of the TopColor
		rCount = (DOUBLE)GetRValue(dwColor1);
		gCount = (DOUBLE)GetGValue(dwColor1);
		bCount = (DOUBLE)GetBValue(dwColor1);
		
		//Calcualte the step of R,G,B values
		rStep = -((DOUBLE)rCount-GetRValue(dwColor2))/NUM_COLORS;
		gStep = -((DOUBLE)gCount-GetGValue(dwColor2))/NUM_COLORS;
		bStep = -((DOUBLE)bCount-GetBValue(dwColor2))/NUM_COLORS;
		
		for (ColorCount=0; ColorCount < NUM_COLORS; ++ColorCount)
		{
			//Draw using current RGB values and Change RGB values
			//to represent the next color in the chain
			dwColor = RGB((DWORD)rCount, (DWORD)gCount, (DWORD)bCount);

			rect.top = dy;
			rect.bottom = dy2;
			rect.left = (LONG)(dx+(ColorCount*RectWidth));
			rect.right = (LONG)(dx+((ColorCount+1)*RectWidth));
			
			// Perform clipping, if needed
			if (bClippingEnabled)
			{
				if (!IntersectRect(&rect, &rect, pClipper))
				{
					rCount += rStep;
					gCount += gStep;
					bCount += bStep;
					
					continue;
				}
			}
			
			// Perform the dithered FillRect()
			if (dwFlags & MGLFILLGRADIENT_OPACITY)
			{
				FillGradientDither565::FillRectOpacity(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					rect.left,
					rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					dwColor,
					dwOpacity);
			}
			else
			{
				FillGradientDither565::FillRect(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					rect.left,
					rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					dwColor);
			}

			rCount += rStep;
			gCount += gStep;
			bCount += bStep;
		}
	}
	else //if (dwFlags & MGLFILLRECTGRADIENT_VERT)
	{
		DOUBLE RectHeight = (dy2 - dy) / 256.0;
		
		//rStep,gStep, and bStep are variables that will be used
		//to hold the values at which R,G,B will be changed respectivily
		//For example: if we have RTop=100 and RBot=150 then 
		//rStep=(150-100)/256 so when we start at R=100 and draw 256 rectangles
		//we will end at R=150 when we finish drawing these rectangles 

		//We will start counting from TopColor to BottomColor
		//So we give an initial value of the TopColor
		rCount = (DOUBLE)GetRValue(dwColor1);
		gCount = (DOUBLE)GetGValue(dwColor1);
		bCount = (DOUBLE)GetBValue(dwColor1);
		
		//Calcualte the step of R,G,B values
		rStep = -((DOUBLE)rCount-GetRValue(dwColor2))/NUM_COLORS;
		gStep = -((DOUBLE)gCount-GetGValue(dwColor2))/NUM_COLORS;
		bStep = -((DOUBLE)bCount-GetBValue(dwColor2))/NUM_COLORS;
		
		for (ColorCount=0; ColorCount < NUM_COLORS; ++ColorCount)
		{
			//Draw using current RGB values and Change RGB values
			//to represent the next color in the chain
			dwColor = RGB((DWORD)rCount, (DWORD)gCount, (DWORD)bCount);

			rect.left = dx;
			rect.right = dx2;
			rect.top = (LONG)(dy+(ColorCount*RectHeight));
			rect.bottom = (LONG)(dy+((ColorCount+1)*RectHeight));
			
			// Perform clipping, if needed
			if (bClippingEnabled)
			{
				if (!IntersectRect(&rect, &rect, pClipper))
				{
					rCount += rStep;
					gCount += gStep;
					bCount += bStep;
					
					continue;
				}
			}
			
			// Perform the dithered FillRect()
			if (dwFlags & MGLFILLGRADIENT_OPACITY)
			{
				FillGradientDither565::FillRectOpacity(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					rect.left,
					rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					dwColor,
					dwOpacity);
			}
			else
			{
				FillGradientDither565::FillRect(dest,
					destXPitch,
					destYPitch,
					destWidth,
					destHeight,
					rect.left,
					rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top,
					dwColor);
			}
			
			rCount += rStep;
			gCount += gStep;
			bCount += bStep;
		}
	}
}
