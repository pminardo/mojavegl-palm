/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "BL_pixelutil.h"
#include "BL_fillrect.h"
#include "BL_fillgradient_twgfx.h"

/**************************************************************
* Internal Functions
**************************************************************/

// Performs a gradient fill; uses TwGfx video hw.
// IMPORTANT NOTE: This function does not support opacity!
void BL_FillGradient_TwGfx(TwGfxSurfaceHandle twDestSurface,
	LONG dx, LONG dy, LONG dx2, LONG dy2, COLORREF dwColor1,
	COLORREF dwColor2, DWORD dwFlags)
{
	DOUBLE rStep, gStep, bStep;
	DOUBLE rCount, gCount, bCount;
	const LONG NUM_COLORS = 256;
	LONG ColorCount;
	TwGfxPackedRGBType twColor;
	RECT rect;
	TwGfxRectType twRect;
	
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
			twColor = TwGfxComponentsToPackedRGB(rCount, gCount, bCount);

			rect.top = dy;
			rect.bottom = dy2;
			rect.left = (LONG)(dx+(ColorCount*RectWidth));
			rect.right = (LONG)(dx+((ColorCount+1)*RectWidth));
			
			// Init twRect
			twRect.x = rect.left;
			twRect.y = rect.top;
			twRect.w = rect.right - rect.left;
			twRect.h = rect.bottom - rect.top;
			
			// Perform the FillRect()
			TwGfxFillRect(twDestSurface, &twRect, twColor);

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
			twColor = TwGfxComponentsToPackedRGB(rCount, gCount, bCount);
			
			rect.left = dx;
			rect.right = dx2;
			rect.top = (LONG)(dy+(ColorCount*RectHeight));
			rect.bottom = (LONG)(dy+((ColorCount+1)*RectHeight));
			
			// Init twRect
			twRect.x = rect.left;
			twRect.y = rect.top;
			twRect.w = rect.right - rect.left;
			twRect.h = rect.bottom - rect.top;
			
			// Perform the FillRect()
			TwGfxFillRect(twDestSurface, &twRect, twColor);
			
			rCount += rStep;
			gCount += gStep;
			bCount += bStep;
		}
	}
}