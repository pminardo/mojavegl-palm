/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_PIXELUTIL_H_
#define __BL_PIXELUTIL_H_

/**************************************************************
* Pixel Utils
**************************************************************/

namespace pxutil {

	// Determines whether the specified surface buffer
	// is aligned for 32-bit blits.
	inline
	BOOL IsAligned32(LONG x, LONG y, LONG xPitch, LONG yPitch, LONG nWidth, LONG nHeight)
	{	
		return (BOOL)((x%2) == 0 && (y%2) == 0 &&
			(max(abs(xPitch), abs(yPitch))%2) /*xyPitch*/ == 0 &&
			(nWidth%2) == 0 && (nHeight%2) == 0);
	}

	inline
	void OptimizePitch(WORD*& pBuffer, LONG& nWidth, LONG& nHeight, LONG& xPitch, LONG& yPitch)
	{
		LONG temp;

		// abs(xPitch) > abs(yPitch)
		if((xPitch>0?xPitch:-xPitch) > (yPitch>0?yPitch:-yPitch)) {
			temp = xPitch;
			xPitch = yPitch;
			yPitch = temp;
			temp = nWidth;
			nWidth = nHeight;
			nHeight = temp;
		}

		if (xPitch < 0) {
			pBuffer += (nWidth - 1) * xPitch;
			xPitch = -xPitch;
		}
	}

	// Computes the average of two alpha/opacity values
	inline
	DWORD AverageAlpha (DWORD alpha, DWORD masterOpacity)
	{
		return (alpha*masterOpacity) >> 8;
	}
	
}

namespace pxutil16bit {

	// Retrieve a pointer to a pixel X, Y location in the specified buffer.
	// NOTE: xPitch and yPitch refer to the number of pixels, **not** the number of bytes!
	inline
	WORD* GetBuffPixels(void *buff, LONG xPitch, LONG yPitch, LONG x, LONG y)
	{
		return (WORD*)buff + (yPitch * y) + /*(x*xPitch)*/x;
	}
}

namespace pxutil565 {

	inline
	DWORD ColorrefToNative(COLORREF color)
	{
	//	m_gxDP.ffFormat & kfDirect565
	//	KfDirect565 = RRRRRGGG.GGGBBBBB
	//	Red         = RRRRRxxx.xxxxxxxx
	//	Green       = xxxxxGGG.GGGxxxxx
	//	Blue        = xxxxxxxx.xxxBBBBB
		return (((color & 0xF8) << 8) | ((color & 0xFC00) >> 5) | ((color & 0xF80000) >> 19));
	}

	inline
	DWORD RGBToNative(DWORD R, DWORD G, DWORD B)
	{
		return (((R & 0xF8) << 8) | ((G & 0xFC) << 3) | ((B & 0xF8) >> 3));
	};

	inline
	COLORREF NativeToColorref(DWORD color)
	{
		// (R) | (G << 8) | (B << 16)
		return ((color & 0xf800) >> 8) | ((color & 0x07e0) << 5) | ((color & 0x001f) << 19);
	}

	inline
	DWORD GetSurfaceRValue(DWORD color)
	{
		return ((color & 0xf800) >> 8); // 5-bit
	}

	inline
	DWORD GetSurfaceGValue(DWORD color)
	{
		return ((color & 0x07e0) >> 3); // 6-bit
	}

	inline
	DWORD GetSurfaceBValue(DWORD color)
	{
		return ((color & 0x001f) << 3); // 5-bit
	}
	
	// Determines the "alpha" value of the specified color,
	// assuming the color came from an alpha mask image.
	inline
	DWORD GetSurfaceAlphaValue(DWORD color)
	{
		// Currently the fastest method is to treat the green
		// component as the alpha value.
		return GetSurfaceGValue(color);
	}
	
	//	KfDirect565 = RRRRRGGG.GGGBBBBB
	//	Divided2    = xRRRRxGG.GGGxBBBB
	inline
	DWORD AlphaBlendFast(DWORD pixel, DWORD backpixel)
	{
		return (((pixel & 0xf7de) >> 1) + ((backpixel & 0xf7de) >> 1));
	}

	// Blend image with background, based on opacity
	inline
	DWORD AlphaBlend(DWORD pixel, DWORD backpixel, DWORD opacity)
	{
		DWORD dw5bitOpacity = opacity >> 3;
	
		//split into xGxRxB
		pixel = (pixel | (pixel<<16)) & 0x07E0F81F; 
		backpixel = (backpixel | (backpixel<<16)) & 0x07E0F81F; 
		
		//do alpha op 
		pixel = ((((pixel-backpixel)*(dw5bitOpacity))>>5) + backpixel) & 0x07E0F81F; 
		
		//recombine into RGB 
		return (WORD)(pixel | pixel >> 16); 
	
		/*DWORD dw5bitOpacity = (opacity-3) >> 3;
		DWORD temp1, temp2;
		
		temp2 = pixel & 0xF81F;
		pixel &= 0x07E0;
		temp1 = backpixel & 0xF81F;
		backpixel &= 0x07E0;

		// Multiply the source by the factor
		backpixel = ((((pixel >> 5) - (backpixel >> 5)) * dw5bitOpacity) + backpixel) & 0x07E0;
		pixel = ((((temp2 - temp1) * dw5bitOpacity) >> 5) + temp1) & 0xF81F;
		
		return backpixel | pixel;*/
	}
	
	inline
	DWORD AlphaBlendAdditive(
		DWORD pixel,
		DWORD backpixel,
		LONG opacity)
	{
		DWORD dw5bitOpacity = opacity >> 3;
		DWORD res;
	
		/* Seperate out the components */
		pixel = ((pixel << 16) | pixel) & 0x7C0F81F;

		/* Multiply the source by the factor */
		pixel = ((pixel * dw5bitOpacity) >> 5) & 0x7C0F81F;
		
		backpixel = ((backpixel << 16) | backpixel) & 0x7C0F81F;
		
		/* Do the conditionless add with saturation */			
		backpixel += pixel;
		res = backpixel & 0x8010020;
		res -= (res >> 5);
		backpixel |= res;
					
		/* Recombine the components */
		backpixel &= 0x7C0F81F;
		backpixel |= (backpixel >> 16);
		backpixel &= 0xFFFF;
		
		return backpixel;
	}
	
	inline
	DWORD AlphaBlendSubtractive(
		DWORD pixel,
		DWORD backpixel,
		LONG opacity)
	{
		DWORD dw5bitOpacity = opacity >> 3;
		DWORD res;
	
		/* Seperate out the components */
		pixel = ((pixel << 16) | pixel) & 0x7C0F81F;

		/* Multiply the source by the factor */
		pixel = ((pixel * dw5bitOpacity) >> 5) & 0x7C0F81F;
		
		backpixel = ((backpixel << 16) | backpixel) & 0x7C0F81F;
		
		/* Do the conditionless add with saturation */			
		backpixel -= pixel;
		res = backpixel & 0x8010020;
		res -= (res >> 5);
		backpixel &= ~res;
					
		/* Recombine the components */
		backpixel &= 0x7C0F81F;
		backpixel |= (backpixel >> 16);
		backpixel &= 0xFFFF;
		
		return backpixel;
	}
	
	// 32-bit fast 50% alpha blending (operates on two pixels simultaneously)
	inline
	DWORD AlphaBlendFast32(
		DWORD pixel,
		DWORD backpixel)
	{
		DWORD dwResult;
			
		dwResult = ((pixel & 0xf7def7de) >> 1) +
			((backpixel & 0xf7def7de) >> 1);
		
		return ((dwResult & 0xFFFF0000) |
			(dwResult & 0x00007FFFF));
	}
	
	// 32-bit alpha blending (operates on two pixels simultaneously)
	inline
	DWORD AlphaBlend32(
		DWORD pixel,
		DWORD backpixel,
		DWORD opacity)
	{
		DWORD dw5bitOpacity = opacity >> 3;
		DWORD temp1, temp2;
		
		temp2 = pixel & 0x7E0F81F;
		pixel &= 0xF81F07E0;
		temp1 = backpixel & 0x7E0F81F;
		backpixel &= 0xF81F07E0;

		// Multiply the source by the factor
		backpixel = ((((pixel >> 5) - (backpixel >> 5)) * dw5bitOpacity) + backpixel) & 0xF81F07E0;
		pixel = ((((temp2 - temp1) * dw5bitOpacity) >> 5) + temp1) & 0x7E0F81F;
		
		return backpixel | pixel;
	}
	
	// NOTE: xPitch and yPitch refer to the number of pixels, **not** the number of bytes!
	inline
	DWORD GetPixel(void *buff, LONG xPitch, LONG yPitch, LONG x, LONG y)
	{
		WORD *pBuff = pxutil16bit::GetBuffPixels(buff, xPitch, yPitch, x, y);
		return *pBuff;
	}
	
	// NOTE: xPitch and yPitch refer to the number of pixels, **not** the number of bytes!
	inline
	void SetPixel(void *buff, LONG xPitch, LONG yPitch, LONG x, LONG y, DWORD dwNativeColor)
	{
		WORD *pBuff = pxutil16bit::GetBuffPixels(buff, xPitch, yPitch, x, y);
		*pBuff = (WORD)dwNativeColor;
	}
	
	// NOTE: xPitch and yPitch refer to the number of pixels, **not** the number of bytes!
	inline
	void SetPixelOpacity(void *buff, LONG xPitch, LONG yPitch, LONG x, LONG y, DWORD dwNativeColor, DWORD dwOpacity)
	{
		WORD *pBuff = pxutil16bit::GetBuffPixels(buff, xPitch, yPitch, x, y);
		*pBuff = (WORD)AlphaBlend(dwNativeColor, *pBuff, dwOpacity);
	}
}

namespace pxutil555 {

	/**
	//	m_gxDP.ffFormat & kfDirect555
	//	KfDirect555 = xRRRRRGG.GGGBBBBB
	//	Red         = xRRRRRxx.xxxxxxxx
	//	Green       = xxxxxxGG.GGGxxxxx
	//	Blue        = xxxxxxxx.xxxBBBBB
	*/
	inline
	DWORD ColorrefToNative(COLORREF color)
	{
		return (((color & 0xF8) << 7) | ((color & 0xF800) >> 6) | ((color & 0xF80000) >> 19));
	};

	inline
	DWORD RGBToNative(DWORD R, DWORD G, DWORD B)
	{
		return (((R & 0xF8) << 7) | ((G & 0xF8) << 2) | ((B & 0xF8) >> 3));
	};

	inline
	COLORREF NativeToColorref(DWORD color)
	{
		// COLORREF = (R) | (G << 8) | (B << 16)
		return ((color & 0x7c00) >> 7) | ((color & 0x03e0) << 6) | ((color & 0x001f) << 19);
	}

	inline
	DWORD GetSurfaceRValue(DWORD color)
	{
		return ((color & 0x7c00) >> 7); // 5-bit
	};

	inline
	DWORD GetSurfaceGValue(DWORD color)
	{
		return ((color & 0x03e0) >> 2); // 5-bit
	};

	inline
	DWORD GetSurfaceBValue(DWORD color)
	{
		return ((color & 0x001f) << 3); // 5-bit
	};


	//	KfDirect555 = xRRRRRGG.GGGBBBBB
	//	Divided2    = xxRRRRxG.GGGxBBBB
	inline
	DWORD AlphaBlendFast(DWORD pixel, DWORD backpixel)
	{
		return (((pixel & 0x7bde) >> 1) + ((backpixel & 0x7bde) >> 1));
	}

	// Blend image with background, based on opacity
	inline
	DWORD AlphaBlend(DWORD pixel, DWORD backpixel, DWORD opacity)
	{
		DWORD dw5bitOpacity = opacity >> 3;
	
		//split into xGxRxB
		pixel = (pixel | (pixel<<16)) & 0x03E07C1F;
		backpixel = (backpixel | (backpixel<<16)) & 0x03E07C1F; 
		
		//do alpha op 
		pixel = ((((pixel-backpixel)*(dw5bitOpacity))>>5) + backpixel) & 0x03E07C1F; 
		
		//recombine into RGB 
		return (pixel | pixel >> 16) & 0x0000FFFFL; 
	
		/*DWORD dw5bitOpacity = opacity >> 3;
		DWORD temp1, temp2;
		
		temp2 = pixel & 0x7C1F;
		pixel &= 0x03E0;
		temp1 = backpixel & 0x7C1F;
		backpixel &= 0x03E0;

		// Multiply the source by the factor
		backpixel = ((((pixel >> 5) - (backpixel >> 5)) * dw5bitOpacity) + backpixel) & 0x03E0;
		pixel = ((((temp2 - temp1) * dw5bitOpacity) >> 5) + temp1) & 0x7C1F;
		
		return backpixel | pixel;*/
	}
	
	inline
	DWORD AlphaBlendAdditive(
		DWORD pixel,
		DWORD backpixel,
		LONG opacity)
	{
		DWORD dw5bitOpacity = opacity >> 3;
		DWORD res;
	
		/* Seperate out the components */
		pixel = ((pixel << 16) | pixel) & 0x3E07C1F;

		/* Multiply the source by the factor */
		pixel = ((pixel * dw5bitOpacity) >> 5) & 0x3E07C1F;
		
		backpixel = ((backpixel << 16) | backpixel) & 0x3E07C1F;
		
		/* Do the conditionless add with saturation */			
		backpixel += pixel;
		res = backpixel & 0x4008020;
		res -= (res >> 5);
		backpixel |= res;
					
		/* Recombine the components */
		backpixel &= 0x3E07C1F;
		backpixel |= (backpixel >> 16);
		backpixel &= 0x7FFF;
		
		return backpixel;
	}
	
	inline
	DWORD AlphaBlendSubtractive(
		DWORD pixel,
		DWORD backpixel,
		LONG opacity)
	{
		DWORD dw5bitOpacity = opacity >> 3;
		DWORD res;
	
		/* Seperate out the components */
		pixel = ((pixel << 16) | pixel) & 0x3E07C1F;

		/* Multiply the source by the factor */
		pixel = ((pixel * dw5bitOpacity) >> 5) & 0x3E07C1F;
		
		backpixel = ((backpixel << 16) | backpixel) & 0x3E07C1F;
		
		/* Do the conditionless add with saturation */			
		backpixel -= pixel;
		res = backpixel & 0x4008020;
		res -= (res >> 5);
		backpixel &= ~res;
					
		/* Recombine the components */
		backpixel &= 0x3E07C1F;
		backpixel |= (backpixel >> 16);
		backpixel &= 0x7FFF;
		
		return backpixel;
	}

	// 32-bit fast 50% alpha blending (operates on two pixels simultaneously)
	inline
	DWORD AlphaBlendFast32(
		DWORD pixel,
		DWORD backpixel)
	{
		DWORD dwResult;
			
		dwResult = ((pixel & 0x7bde7bde) >> 1) +
			((backpixel & 0x7bde7bde) >> 1);
		
		return ((dwResult & 0xFFFF0000) |
			(dwResult & 0x00007FFFF));
	}
	
	// 32-bit alpha blending (operates on two pixels simultaneously)
	inline
	DWORD AlphaBlend32(
		DWORD pixel,
		DWORD backpixel,
		DWORD opacity)
	{
		DWORD dw5bitOpacity = opacity >> 3;
		DWORD temp1, temp2;
		
		temp2 = pixel & 0x3E07C1F;
		pixel &= 0x7C1F03E0;
		temp1 = backpixel & 0x3E07C1F;
		backpixel &= 0x7C1F03E0;

		// Multiply the source by the factor
		backpixel = ((((pixel >> 5) - (backpixel >> 5)) * dw5bitOpacity) + backpixel) & 0x7C1F03E0;
		pixel = ((((temp2 - temp1) * dw5bitOpacity) >> 5) + temp1) & 0x3E07C1F;
		
		return backpixel | pixel;
	}
	
	// NOTE: xPitch and yPitch refer to the number of pixels, **not** the number of bytes!
	inline
	DWORD GetPixel(void *buff, LONG xPitch, LONG yPitch, LONG x, LONG y)
	{
		WORD *pBuff = pxutil16bit::GetBuffPixels(buff, xPitch, yPitch, x, y);
		return *pBuff;
	}
	
	// NOTE: xPitch and yPitch refer to the number of pixels, **not** the number of bytes!
	inline
	void SetPixel(void *buff, LONG xPitch, LONG yPitch, LONG x, LONG y, DWORD dwNativeColor)
	{
		WORD *pBuff = pxutil16bit::GetBuffPixels(buff, xPitch, yPitch, x, y);
		*pBuff = (WORD)dwNativeColor;
	}
	
	// NOTE: xPitch and yPitch refer to the number of pixels, **not** the number of bytes!
	inline
	void SetPixelOpacity(void *buff, LONG xPitch, LONG yPitch, LONG x, LONG y, DWORD dwNativeColor, DWORD dwOpacity)
	{
		WORD *pBuff = pxutil16bit::GetBuffPixels(buff, xPitch, yPitch, x, y);
		*pBuff = (WORD)AlphaBlend(dwNativeColor, *pBuff, dwOpacity);
	}
}

#endif // __BL_PIXELUTIL_H_
