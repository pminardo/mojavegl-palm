/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
*
* NOTES: MojaveGL is FAR from perfect, but it's
* so heavily optimized that I'm happy (for now). One
* thing that bothers me is the fact that OptimizePitch()
* is called called pretty much during every single blit operation...
* it was implemented to make Windows ports
* easier, but it doesn't matter for Palm OS since the surfaces
* are stored in a cache-friendly way to begin with. Plus, it
* complicates the blits because we can't write (*dst++ = *src++).
* Ideally, we should deal with the funky xPitch stuff *ONLY*
* in the Flip() function. It's unfortunate that I didn't realize this
* until MojaveGL was almost finished. Live and learn!
* But, honestly - xPitch has got to go!
**************************************************************/

#include "MojaveGL.h"

#include "blitter/BL_pixelutil.h"

// Blitters
#include "blitter/BL_bitblt.h"
#include "blitter/BL_bitbltopacity.h"
#include "blitter/BL_tilebitblt.h"
#include "blitter/BL_tilebitbltopacity.h"
#include "blitter/BL_stretchblt.h"
#include "blitter/BL_stretchbltopacity.h"
#include "blitter/BL_rotateblt.h"
#include "blitter/BL_rotatebltopacity.h"
#include "blitter/BL_alphablt.h"
#include "blitter/BL_alphabltadditive.h"
#include "blitter/BL_alphabltsubtractive.h"
#include "blitter/BL_stretchalphablt.h"
#include "blitter/BL_stretchalphabltadditive.h"
#include "blitter/BL_stretchalphabltsubtractive.h"
#include "blitter/BL_getpixel.h"
#include "blitter/BL_setpixel.h"
#include "blitter/BL_drawline.h"
#include "blitter/BL_drawrect.h"
#include "blitter/BL_fillrect.h"
#include "blitter/BL_fillgradient.h"
#include "blitter/BL_fillgradient_twgfx.h"
#include "blitter/BL_drawellipse.h"
#include "blitter/BL_fillellipse.h"
#include "blitter/BL_drawpolyline.h"
#include "blitter/BL_drawpolygon.h"
#include "blitter/BL_fillpolygon.h"
#include "blitter/BL_drawbezier.h"

// Fonts
#include "ftfont/FT_defines.h"
#include "ftfont/FT_font.h"

/**************************************************************
* Constants/Macros
**************************************************************/

#define MGL_SURFACE_FTR_CREATOR		'MGL2'

// The image decoder uses this macro to ensure
// the proper endianness of the pixel.
#ifndef __LITTLE_ENDIAN
#define MAKE_PIXEL16(pixel)		(ByteSwap16(pixel))
#else
#define MAKE_PIXEL16(pixel)		(pixel)
#endif

// The following macro is used to compute the opacity
// of a shadow for DrawText().
#define GET_DT_SHADOW_OPACITY(opacity)	(opacity-(opacity/3))

/**************************************************************
* Globals
**************************************************************/

extern TwGfxHandle g_hTwGfx;	// from MglDisplay.cpp
static WORD g_wNextSurfaceFtrNum = 0;

/**************************************************************
* Inline Helper Functions
**************************************************************/

inline
HRESULT DecodeBmp(DWORD dwSurfaceFlags, DWORD dwDestPixelFormat,
	void *dest, LONG destXPitch, LONG destYPitch, LONG destWidth, LONG destHeight,
	BITMAPFILEHEADER *pFileHeader, BITMAPINFOHEADER *pInfoHeader,
	LONG nSrcBytesPerRow, void *bmp)
{
	HRESULT hr;
	DWORD dwDestBuffSize;
	void *pDecodeBuff; 
	LONG row, column;
	DWORD dwPixel;

	// Pixel formats currently supported are RGB555 and RGB565.
	if (dwDestPixelFormat != MGLPIXELFORMAT_555 &&
		dwDestPixelFormat != MGLPIXELFORMAT_565)
	return MGLERR_UNSUPPORTEDMODE;
	
	// Compute the destination buffer size. Alignment
	// of surfaces may vary so always use 'destYPitch'!
	dwDestBuffSize = destYPitch * destHeight;

	// Allocate a buffer to store destination pixels. The buffer
	// makes the code faster because we can consolidate
	// writes to read-only surfaces (only a single DmWrite() call).
	pDecodeBuff = mgl_malloc(dwDestBuffSize);
	if (!pDecodeBuff) return MGLERR_OUTOFMEMORY;
	
	switch (pInfoHeader->biBitCount)
	{
		case 1:
			{
				BITMAPINFO *pBitmapInfo;
				RGBQUAD *pColorTable;
				BYTE *pDestPixels;
				BYTE *pSrcPixels;
				WORD *pDest;
				BYTE *pSrc;
				DWORD dwCurrRowPixel, dwRowBytes, k;
				
				// Set up a pointer that will allow us to access the
				// bitmap's color table. Note that we MUST align
				// the colortable on a 16-bit boundary.
				pBitmapInfo = (BITMAPINFO*)bmp;
				pColorTable = (RGBQUAD*)((WORD*)&pBitmapInfo->bmiColors[3]+1);
				
				// Get a ptr to the dest and source pixels
				pDestPixels = (BYTE*)pDecodeBuff;
				pSrcPixels = (BYTE*)bmp + pFileHeader->bfOffBits;
				dwRowBytes = pInfoHeader->biWidth;
		
				// Factor in padding
				if (pInfoHeader->biWidth%2) ++dwRowBytes;
				
				for (row = 0; row < destHeight; row++)
				{
					pDest = (WORD*) (pDestPixels + row * destYPitch);
					pSrc = (BYTE*) (pSrcPixels + 
						((destHeight - 1) - row) * nSrcBytesPerRow);

					for (dwCurrRowPixel = 0; dwCurrRowPixel < dwRowBytes; )
					{
						// Extract from the next byte
						for (k = 0; k < 8; k++)
						{
							BOOL bBit = (*pSrc >> (7-k)) & 0x01;
							DWORD dwColor = (bBit) ? 0xFF : 0x00;
							
							switch (dwDestPixelFormat)
							{
								case MGLPIXELFORMAT_555:
									dwPixel = pxutil555::RGBToNative(dwColor,
										dwColor, dwColor);
									break;
									
								case MGLPIXELFORMAT_565:
									dwPixel = pxutil565::RGBToNative(dwColor,
										dwColor, dwColor);
									break;
							}
							
							*pDest++ = MAKE_PIXEL16((WORD)dwPixel);
							++dwCurrRowPixel;
							
							// --------------------------------

							// No row pixels left?
							if (dwCurrRowPixel >= dwRowBytes) break;
						}

					++pSrc;
					}
				}
			}
			break;
	
		case 4:
			{
				BITMAPINFO *pBitmapInfo;
				RGBQUAD *pColorTable;
				BYTE *pDestPixels;
				BYTE *pSrcPixels;
				WORD *pDest;
				BYTE *pSrc;
				BYTE colorIndex;
				DWORD dwCurrRowPixel, dwRowBytes;
				
				// Set up a pointer that will allow us to access the
				// bitmap's color table. Note that we MUST align
				// the colortable on a 16-bit boundary.
				pBitmapInfo = (BITMAPINFO*)bmp;
				pColorTable = (RGBQUAD*)((WORD*)&pBitmapInfo->bmiColors[3]+1);
				
				// Get a ptr to the dest and source pixels
				pDestPixels = (BYTE*)pDecodeBuff;
				pSrcPixels = (BYTE*)bmp + pFileHeader->bfOffBits;
				dwRowBytes = pInfoHeader->biWidth;
		
				// Factor in padding
				if (pInfoHeader->biWidth%2) ++dwRowBytes;
				
				for (row = 0; row < destHeight; row++)
				{
					pDest = (WORD*) (pDestPixels + row * destYPitch);
					pSrc = (BYTE*) (pSrcPixels + 
						((destHeight - 1) - row) * nSrcBytesPerRow);

					for (dwCurrRowPixel = 0; dwCurrRowPixel < dwRowBytes; )
					{
						RGBQUAD *pColor;

						// --------------------------------
						// Extract the "high" pixel
						// --------------------------------
						colorIndex = (*pSrc & 0xF0) >> 4; // high pixel
						pColor = &pColorTable[colorIndex];

						switch (dwDestPixelFormat)
						{
							case MGLPIXELFORMAT_555:
								dwPixel = pxutil555::RGBToNative(pColor->rgbRed,
									pColor->rgbGreen, pColor->rgbBlue);
								break;
								
							case MGLPIXELFORMAT_565:
								dwPixel = pxutil565::RGBToNative(pColor->rgbRed,
									pColor->rgbGreen, pColor->rgbBlue);
								break;
						}
						
						*pDest++ = MAKE_PIXEL16((WORD)dwPixel);
						++dwCurrRowPixel;
						
						// --------------------------------

						// No row pixels left?
						if (dwCurrRowPixel >= dwRowBytes) break;
	
						// --------------------------------
						// Extract the "low" pixel
						// --------------------------------
						colorIndex = (*pSrc & 0x0F); // low pixel
						pColor = &pColorTable[colorIndex];

						switch (dwDestPixelFormat)
						{
							case MGLPIXELFORMAT_555:
								dwPixel = pxutil555::RGBToNative(pColor->rgbRed,
									pColor->rgbGreen, pColor->rgbBlue);
								break;
								
							case MGLPIXELFORMAT_565:
								dwPixel = pxutil565::RGBToNative(pColor->rgbRed,
									pColor->rgbGreen, pColor->rgbBlue);
								break;
						}
						
						*pDest++ = MAKE_PIXEL16((WORD)dwPixel);
						++dwCurrRowPixel;
						
						// --------------------------------
						
						++pSrc;
					}
				}
			}
			break;
		
		case 8:
			{
				BITMAPINFO *pBitmapInfo;
				RGBQUAD *pColorTable;
				BYTE *pDestPixels;
				BYTE *pSrcPixels;
				WORD *pDest;
				BYTE *pSrc;
				
				// Set up a pointer that will allow us to access the
				// bitmap's color table. Note that we MUST align
				// the colortable on a 16-bit boundary.
				pBitmapInfo = (BITMAPINFO*)bmp;
				pColorTable = (RGBQUAD*)((WORD*)&pBitmapInfo->bmiColors[3]+1);
				
				// Get a ptr to the dest and source pixels
				pDestPixels = (BYTE*)pDecodeBuff;
				pSrcPixels = (BYTE*)bmp + pFileHeader->bfOffBits;
				
				for (row = 0; row < destHeight; row++)
				{
					pDest = (WORD*) (pDestPixels + row * destYPitch);
					pSrc = (BYTE*) (pSrcPixels + 
						((destHeight - 1) - row) * nSrcBytesPerRow);

					for (column = 0; column < destWidth; column++)
					{
						// Color format is indexed 8-bit
						RGBQUAD *pColor = &pColorTable[*pSrc++];
						
						switch (dwDestPixelFormat)
						{
							case MGLPIXELFORMAT_555:
								dwPixel = pxutil555::RGBToNative(pColor->rgbRed,
									pColor->rgbGreen, pColor->rgbBlue);
								break;
								
							case MGLPIXELFORMAT_565:
								dwPixel = pxutil565::RGBToNative(pColor->rgbRed,
									pColor->rgbGreen, pColor->rgbBlue);
								break;
						}
						
						*pDest++ = MAKE_PIXEL16((WORD)dwPixel);
					}
				}
			}
			break;
	
		case 24:
			{
				BYTE *pDestPixels;
				BYTE *pSrcPixels;
				WORD *pDest;
				BYTE *pSrc;
				
				// Get a ptr to the dest and source pixels
				pDestPixels = (BYTE*)pDecodeBuff;
				pSrcPixels = (BYTE*)bmp + pFileHeader->bfOffBits;
				
				for (row = 0; row < destHeight; row++)
				{
					pDest = (WORD*) (pDestPixels + row * destYPitch);
					pSrc = pSrcPixels + ((destHeight - 1) - row) * nSrcBytesPerRow;

					for (column = 0; column < destWidth; column++)
					{
						DWORD dwBlue, dwGreen, dwRed;
						
						// Color format is BGR888
						dwBlue = *pSrc++;
						dwGreen = *pSrc++;
						dwRed = *pSrc++;
					
						switch (dwDestPixelFormat)
						{
							case MGLPIXELFORMAT_555:
								dwPixel = pxutil555::RGBToNative(dwRed, dwGreen, dwBlue);
								break;
								
							case MGLPIXELFORMAT_565:
								dwPixel = pxutil565::RGBToNative(dwRed, dwGreen, dwBlue);
								break;
						}
						
						*pDest++ = MAKE_PIXEL16((WORD)dwPixel);
					}
				}
			}
			break;
			
		default:
			hr = MGLERR_INVALIDBITMAP;
			goto error;
	}
	
	// Write decoding buffer to actual destination buffer
	if (dwSurfaceFlags & MGLSURFACE_READONLY)
		DmWrite(dest, 0, pDecodeBuff, dwDestBuffSize);
	else
		mgl_memcpy(dest, pDecodeBuff, dwDestBuffSize);
	
	// Free decoding buffer
	mgl_free(pDecodeBuff);

	return MGL_OK;
	
	error:
	
	// Free decoding buffer
	mgl_free(pDecodeBuff);
	
	return hr;
}

/**************************************************************
* Internal Functions
**************************************************************/

// CMglSurface constructor
CMglSurface::CMglSurface(CMgl* pMgl)
{
	m_pMgl = pMgl;
	mgl_memset(&m_surface, 0, sizeof(mglinternal::MGLSURFACE));
}

// CMglSurface destructor
CMglSurface::~CMglSurface()
{
	// Free resources allocated by the surface
	FreeSurface();
}

// Creates a new surface as a copy of the specified source surface
HRESULT CMglSurface::CreateSurface(CMglSurface* pSrcSurface)
{
	DWORD dwFlags;
	HRESULT hr;
	
	// Verify parameters
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	hr = pSrcSurface->GetSurfaceFlags(&dwFlags);
	if (FAILED(hr)) return hr;

	// If either this surface or the source surface is locked,
	// bail out.
	if ((m_surface.dwFlags & MGLSURFACE_LOCKED) ||
		(dwFlags & MGLSURFACE_LOCKED))
		return MGLERR_LOCKEDSURFACES;
		
	// If the source surface isn't allocated, we have a problem...
	if (pSrcSurface->m_surface.dwSize == 0)
		return MGLERR_INVALIDSURFACETYPE;
	
	// Create the empty surface
	hr = CreateSurface(dwFlags,
		pSrcSurface->GetWidth(),
		pSrcSurface->GetHeight());
	if (FAILED(hr)) return hr;
	
	// Bitblt data from src to dest
	BitBlt(0, 0, pSrcSurface, NULL, 0, NULL);

	return MGL_OK;
}

// Creates a new surface of the specified size
HRESULT CMglSurface::CreateSurface(DWORD dwFlags, DWORD dwWidth, DWORD dwHeight)
{
	TwGfxSurfaceInfoType twSurfDesc;
	TwGfxSurfaceHandle twSurfHandle;
	const DWORD dwPixelSize = sizeof(WORD);
	DWORD dwRowBytes, dwAllocSize;
	void *pData;
	Err err;
	
	// If the surface is already allocated...
	if (m_surface.dwSize > 0)
	{
		// Don't reallocate the surface if width & height haven't changed
		if (dwWidth == m_surface.dwWidth &&
			dwHeight == m_surface.dwHeight)
		{
			return MGL_OK;
		}
		
		// Free the surface
		FreeSurface();
		
		// If either the new width & height is zero,
		// we're done!
		if (dwWidth == 0 || dwHeight == 0)
			return MGL_OK;
	}
	else
	{
		// Surface is NOT allocated. Verify width & height.
		if (dwWidth == 0 || dwHeight == 0)
			return MGLERR_INVALIDPARAMS;
	}
	
	// Create the surface in system memory, if requested
	if (dwFlags & MGLSURFACE_SYSTEMMEMORY)
	{
		// Compute the size
		dwRowBytes = dwWidth * dwPixelSize;
		dwAllocSize = dwRowBytes * dwHeight;
	
		// Create in storage memory or dynamic heap?
		if (dwFlags & MGLSURFACE_READONLY)
		{
			// Allocate in storage heap
			err = FtrPtrNew(MGL_SURFACE_FTR_CREATOR,
				g_wNextSurfaceFtrNum,
				dwAllocSize,
				&pData);
			
			// Check for error
			if (err != errNone)
			{
				if (err == memErrNotEnoughSpace)
					return MGLERR_OUTOFMEMORY;
				else
					return MGLERR_INVALIDPARAMS;
			}
			
			// Init surface internals
			m_surface.dwFlags = dwFlags;
			m_surface.dwWidth = dwWidth;
			m_surface.dwHeight = dwHeight;
			m_surface.xPitch = dwPixelSize;
			m_surface.yPitch = dwRowBytes;
			m_surface.dwSize = dwAllocSize;
			m_surface.pBuffer = pData;
			
			// Store ftr memory-specific stuff
			m_surface.dwCreatorID = MGL_SURFACE_FTR_CREATOR;
			m_surface.ftrNum = g_wNextSurfaceFtrNum;
			
			// Adjust surface ftr num
			++g_wNextSurfaceFtrNum;
			
			// Clear the surface, if requested
			if (dwFlags & MGLSURFACE_CLEAR)
				DmSet(pData, 0, dwAllocSize, 0);
		}
		else
		{
			// Allocate in dynamic heap.
			pData = mgl_malloc(dwAllocSize);
			if (!pData) return MGLERR_OUTOFMEMORY;
			
			// Init surface internals
			m_surface.dwFlags = dwFlags;
			m_surface.dwWidth = dwWidth;
			m_surface.dwHeight = dwHeight;
			m_surface.xPitch = dwPixelSize;
			m_surface.yPitch = dwRowBytes;
			m_surface.dwSize = dwAllocSize;
			m_surface.pBuffer = pData;
			
			// Clear the surface, if requested
			if (dwFlags & MGLSURFACE_CLEAR)
				mgl_memset(pData, 0, dwAllocSize);
		}
	}
	// Otherwise, use video memory by default
	else
	{
		// Allocate in local video memory (no backing store).
		// Non-local video memory isn't supported on Tapwave devices.
		dwFlags |= MGLSURFACE_VIDEOMEMORY;
		dwFlags |= MGLSURFACE_LOCALVIDMEM;
		
		twSurfDesc.size = sizeof(TwGfxSurfaceInfoType);
		twSurfDesc.width = (Int32)dwWidth;
		twSurfDesc.height = (Int32)dwHeight;
		twSurfDesc.location = twGfxLocationAcceleratorMemory;
		twSurfDesc.pixelFormat = twGfxPixelFormatRGB565;
		
		err = TwGfxAllocSurface(g_hTwGfx, &twSurfHandle, &twSurfDesc);
		if (err != errNone)
		{
			if (err == twGfxErrorSurfaceAllocFailed)
				return MGLERR_OUTOFMEMORY;
			else
				return MGLERR_INVALIDPARAMS;
		}
		
		// Init surface internals
		m_surface.dwFlags = dwFlags;
		m_surface.dwWidth = dwWidth;
		m_surface.dwHeight = dwHeight;
		m_surface.xPitch = dwPixelSize;
		m_surface.yPitch = twSurfDesc.rowBytes;
		m_surface.dwSize = twSurfDesc.rowBytes * dwHeight;
		m_surface.pBuffer = (void*)twSurfHandle;
		
		// Clear the surface, if requested
		if (dwFlags & MGLSURFACE_CLEAR)
		{
			TwGfxRectType twFillRect;
			
			twFillRect.x = 0;
			twFillRect.y = 0;
			twFillRect.w = dwWidth;
			twFillRect.h = dwHeight;
			
			// Fill with black
			TwGfxFillRect(twSurfHandle, &twFillRect,
				(dwFlags & 0x8000) ? TwGfxComponentsToPackedRGB(255, 0, 0) : TwGfxComponentsToPackedRGB(0, 0, 0));
				
			if (dwFlags & 0x8000)
				TwGfxDrawRect(twSurfHandle, &twFillRect, TwGfxComponentsToPackedRGB(0, 0, 255));
		}
	}

	return MGL_OK;
}

// Creates a new surface from the specified image file in memory.
// This function currently only supports *.bmp and *.dib file formats.
HRESULT CMglSurface::CreateSurface(DWORD dwFlags, BYTE* pImageFileMem, DWORD dwImageFileSize)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	HRESULT hr;
	MGLBUFFDESC buffDesc;
	LONG nSrcBytesPerRow;
	WORD *pw;
	
	// Verify image parameters
	if (!pImageFileMem || dwImageFileSize == 0) return MGLERR_INVALIDPARAMS;
	
	// Ensure there's plenty of image data to read
	// the bitmap headers.
	if (dwImageFileSize < SIZEOF_BITMAPFILEHEADER+sizeof(BITMAPINFOHEADER))
		return MGLERR_INVALIDBITMAP;
		
	// Copy bitmap file data into header structures
	
	// Directly read the WORD-aligned BITMAPFILEHEADER to avoid
	// padding/alignment issues.
	pw = (WORD*)pImageFileMem;
	fileHeader.bfType = pw[0];
	fileHeader.bfSize = pw[1] | pw[2] << 16;
	fileHeader.bfReserved1 = 0;
         fileHeader.bfReserved2 = 0;
         fileHeader.bfOffBits = pw[5] | pw[6] << 16;
	
	mgl_memcpy(&infoHeader, pImageFileMem+SIZEOF_BITMAPFILEHEADER,
		sizeof(BITMAPINFOHEADER));
	
	// -------------------------------------------------
	// Byteswap header structures (if needed)
	
	#ifndef __LITTLE_ENDIAN
	fileHeader.bfType = ByteSwap16(fileHeader.bfType);
	fileHeader.bfSize = ByteSwap32(fileHeader.bfSize);
	fileHeader.bfOffBits = ByteSwap32(fileHeader.bfOffBits);
	
	infoHeader.biSize = ByteSwap32(infoHeader.biSize);
	infoHeader.biWidth = ByteSwap32(infoHeader.biWidth);
	infoHeader.biHeight = ByteSwap32(infoHeader.biHeight);
	infoHeader.biPlanes = ByteSwap16(infoHeader.biPlanes);
	infoHeader.biBitCount = ByteSwap16(infoHeader.biBitCount);
	infoHeader.biCompression = ByteSwap32(infoHeader.biCompression);
	infoHeader.biSizeImage = ByteSwap32(infoHeader.biSizeImage);
	infoHeader.biXPelsPerMeter = ByteSwap32(infoHeader.biXPelsPerMeter);
	infoHeader.biYPelsPerMeter = ByteSwap32(infoHeader.biYPelsPerMeter);
	infoHeader.biClrUsed = ByteSwap32(infoHeader.biClrUsed);
	infoHeader.biClrImportant = ByteSwap32(infoHeader.biClrImportant);
	#endif
	
	// -------------------------------------------------
	
	// Ensure bfType is valid
	if (fileHeader.bfType != 'MB' /* little endian */)
		return MGLERR_INVALIDBITMAP;
	
	// Determine the image format. RLE-encoded BMP, PNG and JPEG
	// formats are not currently supported.
	if (infoHeader.biCompression != BI_RGB)
		return MGLERR_INVALIDBITMAP;
		
	// Validate the bitmap's width and height. CreateSurface() takes in unsigned
	// values, so these must not be negative.
	if (infoHeader.biWidth < 0 || infoHeader.biHeight < 0)
		return MGLERR_INVALIDBITMAP;
	
	// Compute the number of bytes per row
	nSrcBytesPerRow = (((infoHeader.biBitCount * infoHeader.biWidth + 7) / 8 + 3) / 4) * 4;
	
	// Verify that enough data is available to decode the entire bitmap
	if (dwImageFileSize < SIZEOF_BITMAPFILEHEADER+sizeof(BITMAPINFOHEADER)+
			(nSrcBytesPerRow * infoHeader.biHeight))
		return MGLERR_INVALIDBITMAP;
		
	// -------------------------------------------------
	// Create a surface to store the decoded bitmap
	// -------------------------------------------------
	hr = CreateSurface(dwFlags, infoHeader.biWidth, infoHeader.biHeight);
	if (FAILED(hr)) return hr;
	
	// Get the surface buffer
	hr = GetBuffer(&buffDesc);
	if (FAILED(hr)) return hr;
	
	// Perform the decoding based on the format
	switch (infoHeader.biCompression)
	{
		case BI_RGB:
			hr = DecodeBmp(dwFlags, buffDesc.dwPixelFormat,
				buffDesc.pBuffer, buffDesc.xPitch, buffDesc.yPitch,
				buffDesc.dwWidth, buffDesc.dwHeight,
				&fileHeader, &infoHeader,
				nSrcBytesPerRow, pImageFileMem);
			break;
			
		default:
			hr = MGLERR_INVALIDBITMAP;
			break;
	}
	
	// Check for a decoding error
	if (FAILED(hr))
	{
		// Release the surface buffer
		ReleaseBuffer();
		
		// Free surface contents
		FreeSurface();
		
		return hr;
	}
	
	// Release the surface buffer
	ReleaseBuffer();
	
	return MGL_OK;
}

// Frees all resources associated with the surface object. This function
// is called automatically by the destructor.
HRESULT CMglSurface::FreeSurface()
{
	// If surface isn't initialized, bail out.
	if (m_surface.dwSize == 0)
		return MGLERR_NOTINITIALIZED;
		
	if (m_surface.dwFlags & MGLSURFACE_SYSTEMMEMORY)
	{
		if (m_surface.dwFlags & MGLSURFACE_READONLY)
		{
			// Free the surface stored in storage memory
			FtrPtrFree(m_surface.dwCreatorID, (WORD)m_surface.ftrNum);
		}
		else
		{
			// Free the surface stored in dynamic memory
			mgl_free(m_surface.pBuffer);
		}
	}
	else
	{
		// Free the surface stored in video memory
		TwGfxFreeSurface((TwGfxSurfaceHandle)m_surface.pBuffer);
	}
	
	// Reset surface internals
	mgl_memset(&m_surface, 0, sizeof(mglinternal::MGLSURFACE));

	return MGL_OK;
}

HRESULT CMglSurface::SetClipper(RECT* pRect)
{
	// Verify surface
	if (m_surface.dwFlags & MGLSURFACE_LOCKED)
		return MGLERR_LOCKEDSURFACES;
		
	if (!pRect)
	{
		m_surface.clipper.left = 0;
		m_surface.clipper.top = 0;
		m_surface.clipper.right = m_surface.dwWidth;
		m_surface.clipper.bottom = m_surface.dwHeight;
	}
	else
	{
		RECT surfaceRect;
		RECT rect;
		
		mgl_memcpy(&rect, pRect, sizeof(RECT));
		
		// Set the clipper, ensuring that
		// the passed rectangle is valid by intersecting
		// it with a rectangle representing the surface.
		SetRect(&surfaceRect, 0, 0, GetWidth(), GetHeight());
		IntersectRect(&m_surface.clipper, &rect, &surfaceRect);
	}
	
	// If surface is stored in video memory,
	// save the clipper in the TwGfx surface.
	// We must do this for hardware accellerated blits.
	if (m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY)
	{
		TwGfxRectType twClip;
		
		twClip.x = m_surface.clipper.left;
		twClip.y = m_surface.clipper.top;
		twClip.w = m_surface.clipper.right - m_surface.clipper.left;
		twClip.h = m_surface.clipper.bottom - m_surface.clipper.top;
		
		TwGfxSetClip((TwGfxSurfaceHandle)m_surface.pBuffer, &twClip);
	}

	return MGL_OK;
}

HRESULT CMglSurface::GetClipper(RECT* pRect)
{
	// Verify parameters
	if (!pRect) return MGLERR_INVALIDPARAMS;

	// Verify surface
	if (m_surface.dwFlags & MGLSURFACE_LOCKED)
		return MGLERR_LOCKEDSURFACES;
		
	// Copy clipper
	mgl_memcpy(pRect, &m_surface.clipper, sizeof(RECT));
		
	return MGL_OK;
}

DWORD CMglSurface::GetWidth()
{
	return m_surface.dwWidth;
}

DWORD CMglSurface::GetHeight()
{
	return m_surface.dwHeight;
}

HRESULT CMglSurface::GetColorKey(COLORREF* pColorKey)
{
	if (!pColorKey) return MGLERR_INVALIDPARAMS;
	
	// Return colorkey
	*pColorKey = m_surface.ckSrcBlt;
	
	return MGL_OK;
}

HRESULT CMglSurface::SetColorKey(COLORREF dwColorKey)
{
	// Set the colorkey
	m_surface.ckSrcBlt = dwColorKey;

	return MGL_OK;
}

HRESULT CMglSurface::GetBuffer(MGLBUFFDESC* pBuffDesc)
{
	Err err;

	// Verify parameters
	if (!pBuffDesc) return MGLERR_INVALIDPARAMS;
	
	// If the surface doesn't contain pixel data,
	// bail out.
	if (m_surface.dwSize == 0)
		return MGLERR_NOTINITIALIZED;
	
	// Verify surface
	if (m_surface.dwFlags & MGLSURFACE_LOCKED)
		return MGLERR_LOCKEDSURFACES;
	
	// If the surface is stored in video memory, lock it for our use if not
	// already locked by LockVideoSurface().
	if ((m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY) &&
		!(m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED))
	{
		err = TwGfxLockSurface((TwGfxSurfaceHandle)m_surface.pBuffer, &m_surface.pVideoAddr);
		if (err != errNone) return MGLERR_INVALIDPARAMS;
	}
	
	// Lock the surface
	m_surface.dwFlags |= MGLSURFACE_LOCKED;
		
	pBuffDesc->dwWidth = m_surface.dwWidth;
	pBuffDesc->dwHeight = m_surface.dwHeight;
	pBuffDesc->xPitch = m_surface.xPitch;
	pBuffDesc->yPitch = m_surface.yPitch;
	
	// Get the buffer pointer
	if (m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY)
		pBuffDesc->pBuffer = m_surface.pVideoAddr;
	else
		pBuffDesc->pBuffer = m_surface.pBuffer;
	
	pBuffDesc->dwPixelFormat = MGLPIXELFORMAT_565;	// PalmOS uses RGB565LE
	
	return MGL_OK;
}

HRESULT CMglSurface::ReleaseBuffer()
{
	// Verify surface
	if (!(m_surface.dwFlags & MGLSURFACE_LOCKED))
		return MGLERR_NOTLOCKED;
		
	// Unlock the surface
	m_surface.dwFlags &= ~MGLSURFACE_LOCKED;
	
	// If the surface is stored in video memory, unlock it if not
	// locked by LockVideoSurface().
	if ((m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY) &&
		!(m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED))
	{
		TwGfxUnlockSurface((TwGfxSurfaceHandle)m_surface.pBuffer, false);
	}
	
	return MGL_OK;
}

HRESULT CMglSurface::GetSurfaceFlags(DWORD* pFlags)
{
	// Verify parameters
	if (!pFlags) return MGLERR_INVALIDPARAMS;
	
	// Return the flags
	*pFlags = m_surface.dwFlags;
	
	return MGL_OK;
}

HRESULT CMglSurface::LockVideoSurface()
{
	Err err;

	// Ensure that the surface is stored in video memory
	if (!(m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY))
		return MGLERR_NOVIDEOSURFACE;
		
	// If surface is already video locked, bail out.
	if (m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED)
		return MGLERR_LOCKEDSURFACES;
		
	// Lock video surface
	err = TwGfxLockSurface((TwGfxSurfaceHandle)m_surface.pBuffer, &m_surface.pVideoAddr);
	if (err != errNone) return MGLERR_INVALIDPARAMS;
	
	// Set video locked flag
	m_surface.dwFlags |= MGLSURFACE_VIDEOLOCKED;

	return MGL_OK;
}

HRESULT CMglSurface::UnlockVideoSurface()
{
	// If surface isn't video locked, bail out.
	if (!(m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED))
		return MGLERR_NOTLOCKED;
	
	// Unlock the video surface
	TwGfxUnlockSurface((TwGfxSurfaceHandle)m_surface.pBuffer, false);

	// Remove video locked flag
	m_surface.dwFlags &= ~MGLSURFACE_VIDEOLOCKED;

	return MGL_OK;
}

HRESULT CMglSurface::BitBlt(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwFlags, MGLBITBLTFX *pBitBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, destSurfaceWidth, destSurfaceHeight,
		destRight, destBottom;
	RECT srcRect, clipper;
	COLORREF dwColorKey, dwFillColor;

	// Verify source surface ptr
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify FX parameter
	if ((dwFlags & MGLBITBLT_COLORFILL) && !pBitBltFx)
		return MGLERR_INVALIDPARAMS;
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get dest surface width & height
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destX   >= destSurfaceWidth) ||
		(destY    >= destSurfaceHeight) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth))
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destRight = destX+srcRect.right-srcRect.left;
	destBottom = destY+srcRect.bottom-srcRect.top;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		if (destX < 0 || destY < 0 ||
			destRight > destSurfaceWidth ||
			destBottom > destSurfaceHeight)
		{
			// TODO: Verify this return code!
			return MGL_OK;
		}
	}
	else
	{
		// Handle clipping (left)
		if (destX < clipper.left)
		{
			srcRect.left += clipper.left-destX;
			destX = clipper.left;
		}
		
		// Handle clipping (right)
		if (destRight > clipper.right)
		{
			srcRect.right -= destRight - clipper.right;
		}
		
		// Handle clipping (top)
		if (destY < clipper.top)
		{
			srcRect.top += clipper.top-destY;
			destY = clipper.top;
		}
		
		// Handle clipping (bottom)
		if (destBottom > clipper.bottom)
		{
			srcRect.bottom -= destBottom - clipper.bottom;
		}
		
		// Make sure that there's something to blit
		if (IsRectEmpty(&srcRect))
			return MGL_OK;
	}
	
	// Retrieve color key, if needed
	if (dwFlags & MGLBITBLT_KEYSRC)
		pSrcSurface->GetColorKey(&dwColorKey);
		
	// Retrieve fill color, if needed
	if (dwFlags & MGLBITBLT_COLORFILL)
		dwFillColor = pBitBltFx->dwFillColor;
	
	// ---------------------------------------------
	// TW Video Surface Blitting
	// ---------------------------------------------
	
	// If both src & dest are located in video memory,
	// use TwGfx hardware blitter, when possible.
	if (!(dwFlags & MGLBITBLT_COLORFILL) &&
		(m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY) &&
		(pSrcSurface->m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY))
	{
		BOOL bDestVideoLocked, bSrcVideoLocked;
		TwGfxPointType twDestPt;
		TwGfxRectType twSrcRect;
			
		bDestVideoLocked = (m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED);
		bSrcVideoLocked = (pSrcSurface->m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED);
	
		// If surfaces are video locked, unlock them.
		if (bDestVideoLocked)	UnlockVideoSurface();
		if (bSrcVideoLocked)	pSrcSurface->UnlockVideoSurface();
	
		twDestPt.x = destX;
		twDestPt.y = destY;
		
		twSrcRect.x = srcRect.left;
		twSrcRect.y = srcRect.top;
		twSrcRect.w = srcRect.right - srcRect.left;
		twSrcRect.h = srcRect.bottom - srcRect.top;
		
		// Perform a color-keyed (transparent) blit, if requested
		if (dwFlags & MGLBITBLT_KEYSRC)
		{
			TwGfxPackedRGBType twTransparentColor;
			twTransparentColor = TwGfxComponentsToPackedRGB(
					GetRValue(dwColorKey),
					GetGValue(dwColorKey),
					GetBValue(dwColorKey)
				);
			
			TwGfxTransparentBlt((TwGfxSurfaceHandle) m_surface.pBuffer,
			                &twDestPt,
			                (TwGfxSurfaceHandle)pSrcSurface->m_surface.pBuffer,
			                &twSrcRect,
		                        twTransparentColor);
		}
		else
		{
			TwGfxBitBlt((TwGfxSurfaceHandle) m_surface.pBuffer,
		                &twDestPt,
		                (TwGfxSurfaceHandle)pSrcSurface->m_surface.pBuffer,
		                &twSrcRect);
		}
		
		// If surfaces were video locked, relock them.
		if (bDestVideoLocked)	LockVideoSurface();
		if (bSrcVideoLocked)	pSrcSurface->LockVideoSurface();
		
		// We're done!
		return MGL_OK;
	}
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	LONG srcRectWidth = srcRect.right - srcRect.left;
	LONG srcRectHeight = srcRect.bottom - srcRect.top;

	if (dwFlags & MGLBITBLT_KEYSRC)
	{
		DWORD dwNativeColorKey = pxutil565::ColorrefToNative(dwColorKey);
	
		if (dwFlags & MGLBITBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
		
			BL_BitBltKeySrcColorFill565(destBuffDesc.pBuffer,
				srcBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				srcXPitch,
				srcYPitch,
				destX,
				destY,
				destSurfaceWidth,
				destSurfaceHeight,
				srcRect.left,
				srcRect.top,
				srcRectWidth,
				srcRectHeight,
				dwNativeColorKey,
				dwNativeColorFill);
		}
		else
		{
			BL_BitBltKeySrc565(destBuffDesc.pBuffer,
				srcBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				srcXPitch,
				srcYPitch,
				destX,
				destY,
				destSurfaceWidth,
				destSurfaceHeight,
				srcRect.left,
				srcRect.top,
				srcRectWidth,
				srcRectHeight,
				dwNativeColorKey);
		}
	}
	else
	{
		if (dwFlags & MGLBITBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
			
			// This part is pretty easy - simple as FillRect16().
			BL_FillRect565(destBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				destSurfaceWidth,
				destSurfaceHeight,
				destX,
				destY,
				srcRectWidth,
				srcRectHeight,
				dwNativeColorFill);
		}
		else
		{
			BL_BitBlt565(destBuffDesc.pBuffer,
				srcBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				srcXPitch,
				srcYPitch,
				destX,
				destY,
				destSurfaceWidth,
				destSurfaceHeight,
				srcRect.left,
				srcRect.top,
				srcRectWidth,
				srcRectHeight);
		}
	}
	
	// -----------------------------------------
	
	// Release the surface buffers
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::BitBltOpacity(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwOpacity, DWORD dwFlags, MGLBITBLTFX *pBitBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, destSurfaceWidth, destSurfaceHeight,
		destRight, destBottom;
	RECT srcRect, clipper;
	COLORREF dwColorKey, dwFillColor;

	// Verify source surface ptr
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify FX parameter
	if ((dwFlags & MGLBITBLT_COLORFILL) && !pBitBltFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't draw if opacity is zero
	if (dwOpacity == OPACITY_0) return MGL_OK;
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get dest surface width & height
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destX   >= destSurfaceWidth) ||
		(destY    >= destSurfaceHeight) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth))
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destRight = destX+srcRect.right-srcRect.left;
	destBottom = destY+srcRect.bottom-srcRect.top;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		if (destX < 0 || destY < 0 ||
			destRight > destSurfaceWidth ||
			destBottom > destSurfaceHeight)
		{
			// TODO: Verify this return code!
			return MGL_OK;
		}
	}
	else
	{
		// Handle clipping (left)
		if (destX < clipper.left)
		{
			srcRect.left += clipper.left-destX;
			destX = clipper.left;
		}
		
		// Handle clipping (right)
		if (destRight > clipper.right)
		{
			srcRect.right -= destRight - clipper.right;
		}
		
		// Handle clipping (top)
		if (destY < clipper.top)
		{
			srcRect.top += clipper.top-destY;
			destY = clipper.top;
		}
		
		// Handle clipping (bottom)
		if (destBottom > clipper.bottom)
		{
			srcRect.bottom -= destBottom - clipper.bottom;
		}
		
		// Make sure that there's something to blit
		if (IsRectEmpty(&srcRect))
			return MGL_OK;
	}
	
	// Retrieve color key, if needed
	if (dwFlags & MGLBITBLT_KEYSRC)
		pSrcSurface->GetColorKey(&dwColorKey);
		
	// Retrieve fill color, if needed
	if (dwFlags & MGLBITBLT_COLORFILL)
		dwFillColor = pBitBltFx->dwFillColor;
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	LONG srcRectWidth = srcRect.right - srcRect.left;
	LONG srcRectHeight = srcRect.bottom - srcRect.top;

	if (dwFlags & MGLBITBLT_KEYSRC)
	{
		DWORD dwNativeColorKey = pxutil565::ColorrefToNative(dwColorKey);
	
		if (dwFlags & MGLBITBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
		
			BL_BitBltOpacityKeySrcColorFill565(destBuffDesc.pBuffer,
				srcBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				srcXPitch,
				srcYPitch,
				destX,
				destY,
				destSurfaceWidth,
				destSurfaceHeight,
				srcRect.left,
				srcRect.top,
				srcRectWidth,
				srcRectHeight,
				dwNativeColorKey,
				dwNativeColorFill,
				dwOpacity);
		}
		else
		{
			BL_BitBltOpacityKeySrc565(destBuffDesc.pBuffer,
				srcBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				srcXPitch,
				srcYPitch,
				destX,
				destY,
				destSurfaceWidth,
				destSurfaceHeight,
				srcRect.left,
				srcRect.top,
				srcRectWidth,
				srcRectHeight,
				dwNativeColorKey,
				dwOpacity);
		}
	}
	else
	{
		if (dwFlags & MGLBITBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
			
			// This part is pretty easy - simple as FillRect16().
			BL_FillRectOpacity565(destBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				destSurfaceWidth,
				destSurfaceHeight,
				destX,
				destY,
				srcRectWidth,
				srcRectHeight,
				dwNativeColorFill,
				dwOpacity);
		}
		else
		{
			BL_BitBltOpacity565(destBuffDesc.pBuffer,
				srcBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				srcXPitch,
				srcYPitch,
				destX,
				destY,
				destSurfaceWidth,
				destSurfaceHeight,
				srcRect.left,
				srcRect.top,
				srcRectWidth,
				srcRectHeight,
				dwOpacity);
		}
	}
	
	// -----------------------------------------
	
	// Release the surface buffers
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::TileBitBlt(RECT *pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwFlags, MGLBITBLTFX *pBitBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, destSurfaceWidth, destSurfaceHeight;
	RECT srcRect, destRect, clipper;
	COLORREF dwColorKey = 0;
	COLORREF dwFillColor = 0;

	// Verify source surface ptr
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify FX parameter
	if ((dwFlags & MGLBITBLT_COLORFILL) && !pBitBltFx)
		return MGLERR_INVALIDPARAMS;
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get destination surface rect
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pDestRect) {
		destRect.left = pDestRect->left;
		destRect.top = pDestRect->top;
		destRect.right = pDestRect->right;
		destRect.bottom = pDestRect->bottom;
	} else {
		destRect.left = 0;
		destRect.top = 0;
		destRect.right = destSurfaceWidth;
		destRect.bottom = destSurfaceHeight;
	}
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of destination rectangle. */
	if ((destRect.right <= destRect.left) || (destRect.bottom <= destRect.top))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destRect.left   >= destSurfaceWidth) ||
		(destRect.top    >= destSurfaceHeight) ||
		(destRect.bottom <= 0) || (destRect.right <= 0) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth))
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the dest rectangle
		// will fit within the destination surface area.
		if (destRect.left < 0 ||
			destRect.top < 0 ||
			destRect.right > destSurfaceWidth ||
			destRect.bottom > destSurfaceHeight)
		{
			return MGLERR_INVALIDRECT;
		}
	}
	else
	{
		// The TileBitBlt() blitter requires a valid clipping
		// rectangle since it ALWAYS clips the tiling.
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}
	
	// Retrieve color key, if needed
	if (dwFlags & MGLBITBLT_KEYSRC)
		pSrcSurface->GetColorKey(&dwColorKey);
		
	// Retrieve fill color, if needed
	if (dwFlags & MGLBITBLT_COLORFILL)
		dwFillColor = pBitBltFx->dwFillColor;
		
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);

	BL_TileBitBlt565(destBuffDesc.pBuffer, destXPitch, destYPitch,
		destSurfaceWidth, destSurfaceHeight, &destRect,
		srcBuffDesc.pBuffer, srcXPitch, srcYPitch, srcSurfaceWidth, srcSurfaceHeight,
		&srcRect, &clipper, dwFlags, dwColorKey, dwFillColor);
	
	// -----------------------------------------
	
	// Release the surface buffers
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::TileBitBltOpacity(RECT *pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwOpacity, DWORD dwFlags, MGLBITBLTFX *pBitBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, destSurfaceWidth, destSurfaceHeight;
	RECT srcRect, destRect, clipper;
	COLORREF dwColorKey = 0;
	COLORREF dwFillColor = 0;

	// Verify source surface ptr
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify FX parameter
	if ((dwFlags & MGLBITBLT_COLORFILL) && !pBitBltFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't draw if opacity is zero
	if (dwOpacity == OPACITY_0) return MGL_OK;
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get destination surface rect
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pDestRect) {
		destRect.left = pDestRect->left;
		destRect.top = pDestRect->top;
		destRect.right = pDestRect->right;
		destRect.bottom = pDestRect->bottom;
	} else {
		destRect.left = 0;
		destRect.top = 0;
		destRect.right = destSurfaceWidth;
		destRect.bottom = destSurfaceHeight;
	}
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of destination rectangle. */
	if ((destRect.right <= destRect.left) || (destRect.bottom <= destRect.top))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destRect.left   >= destSurfaceWidth) ||
		(destRect.top    >= destSurfaceHeight) ||
		(destRect.bottom <= 0) || (destRect.right <= 0) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth))
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the dest rectangle
		// will fit within the destination surface area.
		if (destRect.left < 0 ||
			destRect.top < 0 ||
			destRect.right > destSurfaceWidth ||
			destRect.bottom > destSurfaceHeight)
		{
			return MGLERR_INVALIDRECT;
		}
	}
	else
	{
		// The TileBitBlt() blitter requires a valid clipping
		// rectangle since it ALWAYS clips the tiling.
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}
	
	// Retrieve color key, if needed
	if (dwFlags & MGLBITBLT_KEYSRC)
		pSrcSurface->GetColorKey(&dwColorKey);
		
	// Retrieve fill color, if needed
	if (dwFlags & MGLBITBLT_COLORFILL)
		dwFillColor = pBitBltFx->dwFillColor;
		
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);

	BL_TileBitBltOpacity565(destBuffDesc.pBuffer, destXPitch, destYPitch,
		destSurfaceWidth, destSurfaceHeight, &destRect,
		srcBuffDesc.pBuffer, srcXPitch, srcYPitch, srcSurfaceWidth, srcSurfaceHeight,
		&srcRect, &clipper, dwFlags, dwColorKey, dwFillColor, dwOpacity);
	
	// -----------------------------------------
	
	// Release the surface buffers
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::StretchBlt(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwFlags, MGLSTRETCHBLTFX* pStretchBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, destSurfaceWidth, destSurfaceHeight;
	RECT srcRect, destRect, clipper;
	COLORREF dwColorKey, dwFillColor;

	// Verify parameters
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify FX parameter
	if ((dwFlags & MGLSTRETCHBLT_COLORFILL) && !pStretchBltFx)
		return MGLERR_INVALIDPARAMS;
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get destination surface rect
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pDestRect) {
		destRect.left = pDestRect->left;
		destRect.top = pDestRect->top;
		destRect.right = pDestRect->right;
		destRect.bottom = pDestRect->bottom;
	} else {
		destRect.left = 0;
		destRect.top = 0;
		destRect.right = destSurfaceWidth;
		destRect.bottom = destSurfaceHeight;
	}
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of destination rectangle. */
	if ((destRect.right <= destRect.left) || (destRect.bottom <= destRect.top))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destRect.left   >= destSurfaceWidth) ||
		(destRect.top    >= destSurfaceHeight) ||
		(destRect.bottom <= 0) || (destRect.right <= 0) ||	// added for StretchBlt()
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth))
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the dest rectangle
		// will fit within the destination surface area.
		if (destRect.left < 0 ||
			destRect.top < 0 ||
			destRect.right > destSurfaceWidth ||
			destRect.bottom > destSurfaceHeight)
		{
			return MGLERR_INVALIDRECT;
		}
	}
	else
	{
		RECT clippedDestRect;
	
		// OPTIMIZATION: Ensure that there's actually something to draw. If there's
		// not, we'll just waste time calling into the ARM code. - ptm
		IntersectRect(&clippedDestRect, &destRect, &clipper);
		
		// Only draw if there's something to draw
		if (IsRectEmpty(&clippedDestRect))
			return MGL_OK;
	}
	
	// Retrieve color key, if needed
	if (dwFlags & MGLSTRETCHBLT_KEYSRC)
		pSrcSurface->GetColorKey(&dwColorKey);
		
	// Retrieve fill color, if needed
	if (dwFlags & MGLSTRETCHBLT_COLORFILL)
		dwFillColor = pStretchBltFx->dwFillColor;
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}

	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLSTRETCHBLT_KEYSRC)
	{
		DWORD dwNativeColorKey = pxutil565::ColorrefToNative(dwColorKey);
	
		if (dwFlags & MGLSTRETCHBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
		
			BL_StretchBltKeySrcColorFill565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				destSurfaceWidth, destSurfaceHeight, &destRect, srcBuffDesc.pBuffer,
				srcXPitch, srcYPitch, &srcRect, &clipper, dwFlags, dwNativeColorKey,
				dwNativeColorFill);
		}
		else
		{
			BL_StretchBltKeySrc565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				destSurfaceWidth, destSurfaceHeight, &destRect, srcBuffDesc.pBuffer,
				srcXPitch, srcYPitch, &srcRect, &clipper, dwFlags, dwNativeColorKey);
		}
	}
	else
	{
		if (dwFlags & MGLSTRETCHBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
			
			// This part is pretty easy - simple as FillRect().
			
			// Clip the fill rectangle for FillRect() if a valid clipper
			// is specified.
			if (!IsRectEmpty(&clipper))
			{
				IntersectRect(&destRect, &destRect, &clipper);
				
				// Only draw if there's something to draw
				if (IsRectEmpty(&destRect)) goto done;
			}
			
			BL_FillRect565(destBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				destSurfaceWidth,
				destSurfaceHeight,
				destRect.left,
				destRect.top,
				destRect.right-destRect.left,
				destRect.bottom-destRect.top,
				dwNativeColorFill);
		}
		else
		{
			BL_StretchBlt565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				destSurfaceWidth, destSurfaceHeight, &destRect, srcBuffDesc.pBuffer,
				srcXPitch, srcYPitch, &srcRect, &clipper, dwFlags);
		}
	}
	
	// -----------------------------------------
	done:
	
	// Release the surface buffers
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::StretchBltOpacity(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwOpacity, DWORD dwFlags, MGLSTRETCHBLTFX* pStretchBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, destSurfaceWidth, destSurfaceHeight;
	RECT srcRect, destRect, clipper;
	COLORREF dwColorKey, dwFillColor;

	// Verify parameters
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify FX parameter
	if ((dwFlags & MGLSTRETCHBLT_COLORFILL) && !pStretchBltFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't draw if opacity is zero
	if (dwOpacity == OPACITY_0) return MGL_OK;
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get destination surface rect
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pDestRect) {
		destRect.left = pDestRect->left;
		destRect.top = pDestRect->top;
		destRect.right = pDestRect->right;
		destRect.bottom = pDestRect->bottom;
	} else {
		destRect.left = 0;
		destRect.top = 0;
		destRect.right = destSurfaceWidth;
		destRect.bottom = destSurfaceHeight;
	}
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of destination rectangle. */
	if ((destRect.right <= destRect.left) || (destRect.bottom <= destRect.top))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destRect.left   >= destSurfaceWidth) ||
		(destRect.top    >= destSurfaceHeight) ||
		(destRect.bottom <= 0) || (destRect.right <= 0) ||	// added for StretchBlt()
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth))
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the dest rectangle
		// will fit within the destination surface area.
		if (destRect.left < 0 ||
			destRect.top < 0 ||
			destRect.right > destSurfaceWidth ||
			destRect.bottom > destSurfaceHeight)
		{
			return MGLERR_INVALIDRECT;
		}
	}
	else
	{
		RECT clippedDestRect;
	
		// OPTIMIZATION: Ensure that there's actually something to draw. If there's
		// not, we'll just waste time calling into the ARM code. - ptm
		IntersectRect(&clippedDestRect, &destRect, &clipper);
		
		// Only draw if there's something to draw
		if (IsRectEmpty(&clippedDestRect))
			return MGL_OK;
	}
	
	// Retrieve color key, if needed
	if (dwFlags & MGLSTRETCHBLT_KEYSRC)
		pSrcSurface->GetColorKey(&dwColorKey);
		
	// Retrieve fill color, if needed
	if (dwFlags & MGLSTRETCHBLT_COLORFILL)
		dwFillColor = pStretchBltFx->dwFillColor;
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}

	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLSTRETCHBLT_KEYSRC)
	{
		DWORD dwNativeColorKey = pxutil565::ColorrefToNative(dwColorKey);
	
		if (dwFlags & MGLSTRETCHBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
		
			BL_StretchBltOpacityKeySrcColorFill565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				destSurfaceWidth, destSurfaceHeight, &destRect, srcBuffDesc.pBuffer,
				srcXPitch, srcYPitch, &srcRect, &clipper, dwFlags, dwNativeColorKey,
				dwNativeColorFill, dwOpacity);
		}
		else
		{
			BL_StretchBltOpacityKeySrc565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				destSurfaceWidth, destSurfaceHeight, &destRect, srcBuffDesc.pBuffer,
				srcXPitch, srcYPitch, &srcRect, &clipper, dwFlags, dwNativeColorKey,
				dwOpacity);
		}
	}
	else
	{
		if (dwFlags & MGLSTRETCHBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
			
			// This part is pretty easy - simple as FillRect().
			
			// Clip the fill rectangle for FillRect() if a valid clipper
			// is specified.
			if (!IsRectEmpty(&clipper))
			{
				IntersectRect(&destRect, &destRect, &clipper);
				
				// Only draw if there's something to draw
				if (IsRectEmpty(&destRect)) goto done;
			}
			
			BL_FillRectOpacity565(destBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				destSurfaceWidth,
				destSurfaceHeight,
				destRect.left,
				destRect.top,
				destRect.right-destRect.left,
				destRect.bottom-destRect.top,
				dwNativeColorFill,
				dwOpacity);
		}
		else
		{
			BL_StretchBltOpacity565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				destSurfaceWidth, destSurfaceHeight, &destRect, srcBuffDesc.pBuffer,
				srcXPitch, srcYPitch, &srcRect, &clipper, dwFlags, dwOpacity);
		}
	}
	
	// -----------------------------------------
	done:
	
	// Release the surface buffers
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::RotateBlt(LONG destX, LONG destY, FLOAT nAngle, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwFlags, MGLROTATEBLTFX* pRotateBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, destSurfaceWidth, destSurfaceHeight,
		srcRectWidth, srcRectHeight;
	RECT srcRect, clipper;
	LONG c, s, w, h;
	DWORD tmatrix[6];
	RECT dr;
	COLORREF dwColorKey, dwFillColor;

	// Verify source surface ptr
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify FX parameter
	if ((dwFlags & MGLROTATEBLT_COLORFILL) && !pRotateBltFx)
		return MGLERR_INVALIDPARAMS;
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get dest surface width & height
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destX   >= destSurfaceWidth) ||
		(destY    >= destSurfaceHeight) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth))
	{
		return MGL_OK;
	}
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	
	// -----------------------------------------
	// Clipping
	// -----------------------------------------
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	if (dwFlags & MGLROTATEBLT_CENTER)
	{
		FLOAT tw = 0.5f * srcRectWidth;
		FLOAT th = 0.5f * srcRectHeight;
		LONG diag = LONG(mglmath::sqrt(tw*tw+th*th) + 1.0f);
		
		SetRect(&dr, destX-diag, destY-diag, destX+diag, destY+diag);
	}
	else
	{
		// Clipping Code
		FLOAT aCos = mglmath::cos(nAngle);
		FLOAT aSin = mglmath::sin(nAngle);

		// Map the four corners and find the bounding rectangle
		FLOAT px[4] = { 0, srcRectWidth, srcRectWidth, 0 };
		FLOAT py[4] = { 0, 0, srcRectHeight, srcRectHeight };
		FLOAT aMinX = 10000000;
		FLOAT aMaxX = -10000000;
		FLOAT aMinY = 10000000;
		FLOAT aMaxY = -10000000;

		for (LONG i=0; i<4; i++)
		{
			FLOAT ox = px[i];
			FLOAT oy = py[i];

			px[i] = (ox*aCos + oy*aSin);
			py[i] = (oy*aCos - ox*aSin);

			if (px[i] < aMinX)
				aMinX = px[i];
			if (px[i] > aMaxX)
				aMaxX = px[i];
			if (py[i] < aMinY)
				aMinY = py[i];
			if (py[i] > aMaxY)
				aMaxY = py[i];
		}

		SetRect(&dr, destX+aMinX, destY+aMinY, destX+aMaxX, destY+aMaxY);
	}
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		if (dr.left < 0 || dr.top < 0 ||
			dr.right > destSurfaceWidth ||
			dr.bottom > destSurfaceHeight)
		{
			// TODO: Verify this return code!
			return MGL_OK;
		}
	}
	else
	{
		IntersectRect(&dr, &dr, &clipper);
		
		// Make sure that there's something to blit
		if (IsRectEmpty(&dr))
			return MGL_OK;
	}
	
	// Compute rotation values
	c = LONG(mglmath::cos( -nAngle ) * 1024.0f);
	s = LONG(mglmath::sin( -nAngle ) * 1024.0f);
	w = srcRectWidth << 9;
	h = srcRectHeight << 9;
	
	tmatrix[0] = c;
	tmatrix[1] = -s;
	tmatrix[2] = s;
	tmatrix[3] = c;
	
	if (dwFlags & MGLROTATEBLT_CENTER)
	{
		tmatrix[4] = -c * destX - s * destY + w;
		tmatrix[5] =  s * destX - c * destY + h;
	}
	else
	{
		tmatrix[4] = -c * destX - s * destY;
		tmatrix[5] =  s * destX - c * destY;
	}

	// Optimization: compensate for fixed point arithmetic in rasterization loops
	tmatrix[4] <<= 6;
	tmatrix[5] <<= 6;
	
	// Retrieve color key, if needed
	if (dwFlags & MGLROTATEBLT_KEYSRC)
		pSrcSurface->GetColorKey(&dwColorKey);
		
	// Retrieve fill color, if needed
	if (dwFlags & MGLROTATEBLT_COLORFILL)
		dwFillColor = pRotateBltFx->dwFillColor;
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLROTATEBLT_KEYSRC)
	{
		DWORD dwNativeColorKey = pxutil565::ColorrefToNative(dwColorKey);
	
		if (dwFlags & MGLROTATEBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
		
			BL_RotateBltKeySrcColorFill565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				&dr, tmatrix, srcBuffDesc.pBuffer, srcXPitch, srcYPitch,
				srcRectWidth, srcRectHeight, dwNativeColorKey, dwNativeColorFill);
		}
		else
		{
			BL_RotateBltKeySrc565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				&dr, tmatrix, srcBuffDesc.pBuffer, srcXPitch, srcYPitch,
				srcRectWidth, srcRectHeight, dwNativeColorKey);
		}
	}
	else
	{
		if (dwFlags & MGLROTATEBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
			
			BL_RotateBltColorFill565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				&dr, tmatrix, srcRectWidth, srcRectHeight, dwNativeColorFill);
		}
		else
		{
			BL_RotateBlt565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				&dr, tmatrix, srcBuffDesc.pBuffer, srcXPitch, srcYPitch,
				srcRectWidth, srcRectHeight);
		}
	}
	
	// -----------------------------------------
	
	// Release the surface buffers
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::RotateBltOpacity(LONG destX, LONG destY, FLOAT nAngle, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwOpacity, DWORD dwFlags, MGLROTATEBLTFX* pRotateBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, destSurfaceWidth, destSurfaceHeight,
		srcRectWidth, srcRectHeight;
	RECT srcRect, clipper;
	LONG c, s, w, h;
	DWORD tmatrix[6];
	RECT dr;
	COLORREF dwColorKey, dwFillColor;

	// Verify source surface ptr
	if (!pSrcSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify FX parameter
	if ((dwFlags & MGLROTATEBLT_COLORFILL) && !pRotateBltFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't draw if opacity is zero
	if (dwOpacity == OPACITY_0) return MGL_OK;
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get dest surface width & height
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destX   >= destSurfaceWidth) ||
		(destY    >= destSurfaceHeight) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth))
	{
		return MGL_OK;
	}
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	
	// -----------------------------------------
	// Clipping
	// -----------------------------------------
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	if (dwFlags & MGLROTATEBLT_CENTER)
	{
		FLOAT tw = 0.5f * srcRectWidth;
		FLOAT th = 0.5f * srcRectHeight;
		LONG diag = LONG(mglmath::sqrt(tw*tw+th*th) + 1.0f);
		
		SetRect(&dr, destX-diag, destY-diag, destX+diag, destY+diag);
	}
	else
	{
		// Clipping Code
		FLOAT aCos = mglmath::cos(nAngle);
		FLOAT aSin = mglmath::sin(nAngle);

		// Map the four corners and find the bounding rectangle
		FLOAT px[4] = { 0, srcRectWidth, srcRectWidth, 0 };
		FLOAT py[4] = { 0, 0, srcRectHeight, srcRectHeight };
		FLOAT aMinX = 10000000;
		FLOAT aMaxX = -10000000;
		FLOAT aMinY = 10000000;
		FLOAT aMaxY = -10000000;

		for (LONG i=0; i<4; i++)
		{
			FLOAT ox = px[i];
			FLOAT oy = py[i];

			px[i] = (ox*aCos + oy*aSin);
			py[i] = (oy*aCos - ox*aSin);

			if (px[i] < aMinX)
				aMinX = px[i];
			if (px[i] > aMaxX)
				aMaxX = px[i];
			if (py[i] < aMinY)
				aMinY = py[i];
			if (py[i] > aMaxY)
				aMaxY = py[i];
		}

		SetRect(&dr, destX+aMinX, destY+aMinY, destX+aMaxX, destY+aMaxY);
	}
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		if (dr.left < 0 || dr.top < 0 ||
			dr.right > destSurfaceWidth ||
			dr.bottom > destSurfaceHeight)
		{
			// TODO: Verify this return code!
			return MGL_OK;
		}
	}
	else
	{
		IntersectRect(&dr, &dr, &clipper);
		
		// Make sure that there's something to blit
		if (IsRectEmpty(&dr))
			return MGL_OK;
	}
	
	// Compute rotation values
	c = LONG(mglmath::cos( -nAngle ) * 1024.0f);
	s = LONG(mglmath::sin( -nAngle ) * 1024.0f);
	w = srcRectWidth << 9;
	h = srcRectHeight << 9;
	
	tmatrix[0] = c;
	tmatrix[1] = -s;
	tmatrix[2] = s;
	tmatrix[3] = c;
	
	if (dwFlags & MGLROTATEBLT_CENTER)
	{
		tmatrix[4] = -c * destX - s * destY + w;
		tmatrix[5] =  s * destX - c * destY + h;
	}
	else
	{
		tmatrix[4] = -c * destX - s * destY;
		tmatrix[5] =  s * destX - c * destY;
	}

	// Optimization: compensate for fixed point arithmetic in rasterization loops
	tmatrix[4] <<= 6;
	tmatrix[5] <<= 6;
	
	// Retrieve color key, if needed
	if (dwFlags & MGLROTATEBLT_KEYSRC)
		pSrcSurface->GetColorKey(&dwColorKey);
		
	// Retrieve fill color, if needed
	if (dwFlags & MGLROTATEBLT_COLORFILL)
		dwFillColor = pRotateBltFx->dwFillColor;
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLROTATEBLT_KEYSRC)
	{
		DWORD dwNativeColorKey = pxutil565::ColorrefToNative(dwColorKey);
	
		if (dwFlags & MGLROTATEBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
		
			BL_RotateBltOpacityKeySrcColorFill565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				&dr, tmatrix, srcBuffDesc.pBuffer, srcXPitch, srcYPitch,
				srcRectWidth, srcRectHeight, dwNativeColorKey, dwNativeColorFill, dwOpacity);
		}
		else
		{
			BL_RotateBltOpacityKeySrc565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				&dr, tmatrix, srcBuffDesc.pBuffer, srcXPitch, srcYPitch,
				srcRectWidth, srcRectHeight, dwNativeColorKey, dwOpacity);
		}
	}
	else
	{
		if (dwFlags & MGLROTATEBLT_COLORFILL)
		{
			DWORD dwNativeColorFill = pxutil565::ColorrefToNative(dwFillColor);
			
			BL_RotateBltOpacityColorFill565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				&dr, tmatrix, srcRectWidth, srcRectHeight, dwNativeColorFill, dwOpacity);
		}
		else
		{
			BL_RotateBltOpacity565(destBuffDesc.pBuffer, destXPitch, destYPitch,
				&dr, tmatrix, srcBuffDesc.pBuffer, srcXPitch, srcYPitch,
				srcRectWidth, srcRectHeight, dwOpacity);
		}
	}
	
	// -----------------------------------------
	
	// Release the surface buffers
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::AlphaBlt(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT *pAlphaRect, DWORD dwFlags, MGLALPHABLTFX *pAlphaBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc, alphaBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, alphaSurfaceWidth, alphaSurfaceHeight,
		destSurfaceWidth, destSurfaceHeight, destRight, destBottom,
		srcRectWidth, srcRectHeight, alphaRectWidth, alphaRectHeight;
	RECT srcRect, alphaRect, clipper;

	// Verify source and alpha surface ptrs
	if (!pSrcSurface || !pAlphaSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify opacity flag
	if ((dwFlags & MGLALPHABLT_OPACITY) && !pAlphaBltFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLALPHABLT_OPACITY) && pAlphaBltFx->dwOpacity == OPACITY_0)
		return MGL_OK;
	
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLALPHABLT_OPACITY) &&
		pAlphaBltFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLALPHABLT_OPACITY;
	}
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get alpha surface rect
	alphaSurfaceWidth = pAlphaSurface->GetWidth();
	alphaSurfaceHeight = pAlphaSurface->GetHeight();
	if (pAlphaRect) {
		alphaRect.left = pAlphaRect->left;
		alphaRect.top = pAlphaRect->top;
		alphaRect.right = pAlphaRect->right;
		alphaRect.bottom = pAlphaRect->bottom;
	} else {
		alphaRect.left = 0;
		alphaRect.top = 0;
		alphaRect.right = alphaSurfaceWidth;
		alphaRect.bottom = alphaSurfaceHeight;
	}
	
	// Get dest surface width & height
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of alpha rectangle. */
	if (((alphaRect.bottom > alphaSurfaceHeight) || (alphaRect.bottom < 0) ||
		(alphaRect.top     > alphaSurfaceHeight) || (alphaRect.top    < 0) ||
		(alphaRect.left    > alphaSurfaceWidth)  || (alphaRect.left   < 0) ||
		(alphaRect.right   > alphaSurfaceWidth)  || (alphaRect.right  < 0) ||
		(alphaRect.right   <= alphaRect.left)               || (alphaRect.bottom <= alphaRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	// Ensure that srcRect doesn't exceed alphaRect;
	// otherwise, we could accidentally read outside
	// of the alpha surface bounds.
	if (alphaRectWidth > srcRectWidth)
		srcRect.right -= alphaRectWidth-srcRectWidth;
		
	if (alphaRectHeight > srcRectHeight)
		srcRect.bottom -= alphaRectHeight-srcRectHeight;
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destX   >= destSurfaceWidth) ||
		(destY    >= destSurfaceHeight) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth) ||
		(alphaRect.bottom <= 0) || (alphaRect.right <= 0)     ||
		(alphaRect.top >= alphaSurfaceHeight) ||
		(alphaRect.left >= alphaSurfaceWidth)
		)
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destRight = destX+srcRect.right-srcRect.left;
	destBottom = destY+srcRect.bottom-srcRect.top;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		if (destX < 0 || destY < 0 ||
			destRight > destSurfaceWidth ||
			destBottom > destSurfaceHeight)
		{
			// TODO: Verify this return code!
			return MGL_OK;
		}
	}
	else
	{
		// Handle clipping (left)
		if (destX < clipper.left)
		{
			srcRect.left += clipper.left-destX;
			alphaRect.left += clipper.left-destX;
			destX = clipper.left;
		}
		
		// Handle clipping (right)
		if (destRight > clipper.right)
		{
			srcRect.right -= destRight - clipper.right;
			alphaRect.right -= destRight - clipper.right;
		}
		
		// Handle clipping (top)
		if (destY < clipper.top)
		{
			srcRect.top += clipper.top-destY;
			alphaRect.top += clipper.top-destY;
			destY = clipper.top;
		}
		
		// Handle clipping (bottom)
		if (destBottom > clipper.bottom)
		{
			srcRect.bottom -= destBottom - clipper.bottom;
			alphaRect.bottom -= destBottom - clipper.bottom;
		}
		
		// Make sure that there's something to blit
		if (IsRectEmpty(&srcRect) || IsRectEmpty(&alphaRect))
			return MGL_OK;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// Get the alpha surface buffer
	hr = pAlphaSurface->GetBuffer(&alphaBuffDesc);
	if (FAILED(hr))
	{
		// Release source surface buffer
		pSrcSurface->ReleaseBuffer();
		
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	LONG alphaXPitch = alphaBuffDesc.xPitch / sizeof(WORD);
	LONG alphaYPitch = alphaBuffDesc.yPitch / sizeof(WORD);
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	if (dwFlags & MGLALPHABLT_OPACITY)
	{
		BL_AlphaBltOpacity565(destBuffDesc.pBuffer,
			srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			srcXPitch,
			srcYPitch,
			alphaXPitch,
			alphaYPitch,
			destX,
			destY,
			destSurfaceWidth,
			destSurfaceHeight,
			srcRect.left,
			srcRect.top,
			srcRectWidth,
			srcRectHeight,
			alphaRect.left,
			alphaRect.top,
			alphaRectWidth,
			alphaRectHeight,
			pAlphaBltFx->dwOpacity);
	}
	else
	{
		BL_AlphaBlt565(destBuffDesc.pBuffer,
			srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			srcXPitch,
			srcYPitch,
			alphaXPitch,
			alphaYPitch,
			destX,
			destY,
			destSurfaceWidth,
			destSurfaceHeight,
			srcRect.left,
			srcRect.top,
			srcRectWidth,
			srcRectHeight,
			alphaRect.left,
			alphaRect.top,
			alphaRectWidth,
			alphaRectHeight);
	}

	// -----------------------------------------
	
	// Release the surface buffers
	pAlphaSurface->ReleaseBuffer();
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::AlphaBltAdditive(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT *pAlphaRect, DWORD dwFlags, MGLALPHABLTFX *pAlphaBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc, alphaBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, alphaSurfaceWidth, alphaSurfaceHeight,
		destSurfaceWidth, destSurfaceHeight, destRight, destBottom,
		srcRectWidth, srcRectHeight, alphaRectWidth, alphaRectHeight;
	RECT srcRect, alphaRect, clipper;

	// Verify source and alpha surface ptrs
	if (!pSrcSurface || !pAlphaSurface) return MGLERR_INVALIDPARAMS;

	// Verify opacity flag
	if ((dwFlags & MGLALPHABLT_OPACITY) && !pAlphaBltFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLALPHABLT_OPACITY) && pAlphaBltFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLALPHABLT_OPACITY) &&
		pAlphaBltFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLALPHABLT_OPACITY;
	}
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get alpha surface rect
	alphaSurfaceWidth = pAlphaSurface->GetWidth();
	alphaSurfaceHeight = pAlphaSurface->GetHeight();
	if (pAlphaRect) {
		alphaRect.left = pAlphaRect->left;
		alphaRect.top = pAlphaRect->top;
		alphaRect.right = pAlphaRect->right;
		alphaRect.bottom = pAlphaRect->bottom;
	} else {
		alphaRect.left = 0;
		alphaRect.top = 0;
		alphaRect.right = alphaSurfaceWidth;
		alphaRect.bottom = alphaSurfaceHeight;
	}
	
	// Get dest surface width & height
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of alpha rectangle. */
	if (((alphaRect.bottom > alphaSurfaceHeight) || (alphaRect.bottom < 0) ||
		(alphaRect.top     > alphaSurfaceHeight) || (alphaRect.top    < 0) ||
		(alphaRect.left    > alphaSurfaceWidth)  || (alphaRect.left   < 0) ||
		(alphaRect.right   > alphaSurfaceWidth)  || (alphaRect.right  < 0) ||
		(alphaRect.right   <= alphaRect.left)               || (alphaRect.bottom <= alphaRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	// Ensure that srcRect doesn't exceed alphaRect;
	// otherwise, we could accidentally read outside
	// of the alpha surface bounds.
	if (alphaRectWidth > srcRectWidth)
		srcRect.right -= alphaRectWidth-srcRectWidth;
		
	if (alphaRectHeight > srcRectHeight)
		srcRect.bottom -= alphaRectHeight-srcRectHeight;
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destX   >= destSurfaceWidth) ||
		(destY    >= destSurfaceHeight) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth) ||
		(alphaRect.bottom <= 0) || (alphaRect.right <= 0)     ||
		(alphaRect.top >= alphaSurfaceHeight) ||
		(alphaRect.left >= alphaSurfaceWidth)
		)
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destRight = destX+srcRect.right-srcRect.left;
	destBottom = destY+srcRect.bottom-srcRect.top;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		if (destX < 0 || destY < 0 ||
			destRight > destSurfaceWidth ||
			destBottom > destSurfaceHeight)
		{
			// TODO: Verify this return code!
			return MGL_OK;
		}
	}
	else
	{
		// Handle clipping (left)
		if (destX < clipper.left)
		{
			srcRect.left += clipper.left-destX;
			alphaRect.left += clipper.left-destX;
			destX = clipper.left;
		}
		
		// Handle clipping (right)
		if (destRight > clipper.right)
		{
			srcRect.right -= destRight - clipper.right;
			alphaRect.right -= destRight - clipper.right;
		}
		
		// Handle clipping (top)
		if (destY < clipper.top)
		{
			srcRect.top += clipper.top-destY;
			alphaRect.top += clipper.top-destY;
			destY = clipper.top;
		}
		
		// Handle clipping (bottom)
		if (destBottom > clipper.bottom)
		{
			srcRect.bottom -= destBottom - clipper.bottom;
			alphaRect.bottom -= destBottom - clipper.bottom;
		}
		
		// Make sure that there's something to blit
		if (IsRectEmpty(&srcRect) || IsRectEmpty(&alphaRect))
			return MGL_OK;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// Get the alpha surface buffer
	hr = pAlphaSurface->GetBuffer(&alphaBuffDesc);
	if (FAILED(hr))
	{
		// Release source surface buffer
		pSrcSurface->ReleaseBuffer();
		
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	LONG alphaXPitch = alphaBuffDesc.xPitch / sizeof(WORD);
	LONG alphaYPitch = alphaBuffDesc.yPitch / sizeof(WORD);
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	if (dwFlags & MGLALPHABLT_OPACITY)
	{
		BL_AlphaBltAdditiveOpacity565(destBuffDesc.pBuffer,
			srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			srcXPitch,
			srcYPitch,
			alphaXPitch,
			alphaYPitch,
			destX,
			destY,
			destSurfaceWidth,
			destSurfaceHeight,
			srcRect.left,
			srcRect.top,
			srcRectWidth,
			srcRectHeight,
			alphaRect.left,
			alphaRect.top,
			alphaRectWidth,
			alphaRectHeight,
			pAlphaBltFx->dwOpacity);
	}
	else
	{
		BL_AlphaBltAdditive565(destBuffDesc.pBuffer,
			srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			srcXPitch,
			srcYPitch,
			alphaXPitch,
			alphaYPitch,
			destX,
			destY,
			destSurfaceWidth,
			destSurfaceHeight,
			srcRect.left,
			srcRect.top,
			srcRectWidth,
			srcRectHeight,
			alphaRect.left,
			alphaRect.top,
			alphaRectWidth,
			alphaRectHeight);
	}

	// -----------------------------------------
	
	// Release the surface buffers
	pAlphaSurface->ReleaseBuffer();
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::AlphaBltSubtractive(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT *pAlphaRect, DWORD dwFlags, MGLALPHABLTFX *pAlphaBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc, alphaBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, alphaSurfaceWidth, alphaSurfaceHeight,
		destSurfaceWidth, destSurfaceHeight, destRight, destBottom,
		srcRectWidth, srcRectHeight, alphaRectWidth, alphaRectHeight;
	RECT srcRect, alphaRect, clipper;

	// Verify source and alpha surface ptrs
	if (!pSrcSurface || !pAlphaSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify opacity flag
	if ((dwFlags & MGLALPHABLT_OPACITY) && !pAlphaBltFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLALPHABLT_OPACITY) && pAlphaBltFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLALPHABLT_OPACITY) &&
		pAlphaBltFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLALPHABLT_OPACITY;
	}
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get alpha surface rect
	alphaSurfaceWidth = pAlphaSurface->GetWidth();
	alphaSurfaceHeight = pAlphaSurface->GetHeight();
	if (pAlphaRect) {
		alphaRect.left = pAlphaRect->left;
		alphaRect.top = pAlphaRect->top;
		alphaRect.right = pAlphaRect->right;
		alphaRect.bottom = pAlphaRect->bottom;
	} else {
		alphaRect.left = 0;
		alphaRect.top = 0;
		alphaRect.right = alphaSurfaceWidth;
		alphaRect.bottom = alphaSurfaceHeight;
	}
	
	// Get dest surface width & height
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of alpha rectangle. */
	if (((alphaRect.bottom > alphaSurfaceHeight) || (alphaRect.bottom < 0) ||
		(alphaRect.top     > alphaSurfaceHeight) || (alphaRect.top    < 0) ||
		(alphaRect.left    > alphaSurfaceWidth)  || (alphaRect.left   < 0) ||
		(alphaRect.right   > alphaSurfaceWidth)  || (alphaRect.right  < 0) ||
		(alphaRect.right   <= alphaRect.left)               || (alphaRect.bottom <= alphaRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	// Ensure that srcRect doesn't exceed alphaRect;
	// otherwise, we could accidentally read outside
	// of the alpha surface bounds.
	if (alphaRectWidth > srcRectWidth)
		srcRect.right -= alphaRectWidth-srcRectWidth;
		
	if (alphaRectHeight > srcRectHeight)
		srcRect.bottom -= alphaRectHeight-srcRectHeight;
	
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destX   >= destSurfaceWidth) ||
		(destY    >= destSurfaceHeight) ||
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth) ||
		(alphaRect.bottom <= 0) || (alphaRect.right <= 0)     ||
		(alphaRect.top >= alphaSurfaceHeight) ||
		(alphaRect.left >= alphaSurfaceWidth)
		)
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destRight = destX+srcRect.right-srcRect.left;
	destBottom = destY+srcRect.bottom-srcRect.top;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		if (destX < 0 || destY < 0 ||
			destRight > destSurfaceWidth ||
			destBottom > destSurfaceHeight)
		{
			// TODO: Verify this return code!
			return MGL_OK;
		}
	}
	else
	{
		// Handle clipping (left)
		if (destX < clipper.left)
		{
			srcRect.left += clipper.left-destX;
			alphaRect.left += clipper.left-destX;
			destX = clipper.left;
		}
		
		// Handle clipping (right)
		if (destRight > clipper.right)
		{
			srcRect.right -= destRight - clipper.right;
			alphaRect.right -= destRight - clipper.right;
		}
		
		// Handle clipping (top)
		if (destY < clipper.top)
		{
			srcRect.top += clipper.top-destY;
			alphaRect.top += clipper.top-destY;
			destY = clipper.top;
		}
		
		// Handle clipping (bottom)
		if (destBottom > clipper.bottom)
		{
			srcRect.bottom -= destBottom - clipper.bottom;
			alphaRect.bottom -= destBottom - clipper.bottom;
		}
		
		// Make sure that there's something to blit
		if (IsRectEmpty(&srcRect) || IsRectEmpty(&alphaRect))
			return MGL_OK;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// Get the alpha surface buffer
	hr = pAlphaSurface->GetBuffer(&alphaBuffDesc);
	if (FAILED(hr))
	{
		// Release source surface buffer
		pSrcSurface->ReleaseBuffer();
		
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	LONG alphaXPitch = alphaBuffDesc.xPitch / sizeof(WORD);
	LONG alphaYPitch = alphaBuffDesc.yPitch / sizeof(WORD);
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	if (dwFlags & MGLALPHABLT_OPACITY)
	{
		BL_AlphaBltSubtractiveOpacity565(destBuffDesc.pBuffer,
			srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			srcXPitch,
			srcYPitch,
			alphaXPitch,
			alphaYPitch,
			destX,
			destY,
			destSurfaceWidth,
			destSurfaceHeight,
			srcRect.left,
			srcRect.top,
			srcRectWidth,
			srcRectHeight,
			alphaRect.left,
			alphaRect.top,
			alphaRectWidth,
			alphaRectHeight,
			pAlphaBltFx->dwOpacity);
	}
	else
	{
		BL_AlphaBltSubtractive565(destBuffDesc.pBuffer,
			srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			srcXPitch,
			srcYPitch,
			alphaXPitch,
			alphaYPitch,
			destX,
			destY,
			destSurfaceWidth,
			destSurfaceHeight,
			srcRect.left,
			srcRect.top,
			srcRectWidth,
			srcRectHeight,
			alphaRect.left,
			alphaRect.top,
			alphaRectWidth,
			alphaRectHeight);
	}

	// -----------------------------------------
	
	// Release the surface buffers
	pAlphaSurface->ReleaseBuffer();
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::StretchAlphaBlt(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT* pAlphaRect, DWORD dwFlags, MGLSTRETCHALPHABLTFX* pStretchAlphaBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc, alphaBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, alphaSurfaceWidth, alphaSurfaceHeight,
		destSurfaceWidth, destSurfaceHeight, srcRectWidth, srcRectHeight,
		alphaRectWidth, alphaRectHeight;
	RECT srcRect, alphaRect, destRect, clipper;

	// Verify source and alpha surface ptrs
	if (!pSrcSurface || !pAlphaSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify opacity flag
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) && !pStretchAlphaBltFx)
		return MGLERR_INVALIDPARAMS;
	
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) &&
		pStretchAlphaBltFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) &&
		pStretchAlphaBltFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLSTRETCHALPHABLT_OPACITY;
	}
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get alpha surface rect
	alphaSurfaceWidth = pAlphaSurface->GetWidth();
	alphaSurfaceHeight = pAlphaSurface->GetHeight();
	if (pAlphaRect) {
		alphaRect.left = pAlphaRect->left;
		alphaRect.top = pAlphaRect->top;
		alphaRect.right = pAlphaRect->right;
		alphaRect.bottom = pAlphaRect->bottom;
	} else {
		alphaRect.left = 0;
		alphaRect.top = 0;
		alphaRect.right = alphaSurfaceWidth;
		alphaRect.bottom = alphaSurfaceHeight;
	}
	
	// Get destination surface rect
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pDestRect) {
		destRect.left = pDestRect->left;
		destRect.top = pDestRect->top;
		destRect.right = pDestRect->right;
		destRect.bottom = pDestRect->bottom;
	} else {
		destRect.left = 0;
		destRect.top = 0;
		destRect.right = destSurfaceWidth;
		destRect.bottom = destSurfaceHeight;
	}
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of alpha rectangle. */
	if (((alphaRect.bottom > alphaSurfaceHeight) || (alphaRect.bottom < 0) ||
		(alphaRect.top     > alphaSurfaceHeight) || (alphaRect.top    < 0) ||
		(alphaRect.left    > alphaSurfaceWidth)  || (alphaRect.left   < 0) ||
		(alphaRect.right   > alphaSurfaceWidth)  || (alphaRect.right  < 0) ||
		(alphaRect.right   <= alphaRect.left)               || (alphaRect.bottom <= alphaRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of destination rectangle. */
	if ((destRect.right <= destRect.left) || (destRect.bottom <= destRect.top))
	{
		return MGLERR_INVALIDRECT;
	}
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	// We want to be able to use the source X & Y positions
	// when reading the mask.  If the alpha mask rectangle
	// is larger than the source rectangle, shrink it so that we
	// don't read outside of the source image. - ptm
	if (alphaRectWidth > srcRectWidth ||
		alphaRectHeight > srcRectHeight)
	{
		if (alphaRectWidth > srcRectWidth)
			alphaRect.right -= alphaRectWidth-srcRectWidth;
			
		if (alphaRectHeight > srcRectHeight)
			alphaRect.bottom -= alphaRectHeight-srcRectHeight;
	}
	// If the alpha mask rectangle is smaller than the source rectangle,
	// shrink the source rectangle so that we don't read outside of
	// the alpha mask.
	else if (alphaRectWidth < srcRectWidth ||
		alphaRectHeight < srcRectHeight)
	{
		if (alphaRectWidth < srcRectWidth)
			srcRect.right -= srcRectWidth-alphaRectWidth;
			
		if (alphaRectHeight < srcRectHeight)
			srcRect.bottom -= srcRectHeight-alphaRectHeight;
	}
		
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destRect.left   >= destSurfaceWidth) ||
		(destRect.top    >= destSurfaceHeight) ||
		(destRect.bottom <= 0) || (destRect.right <= 0) ||	// added for StretchBlt()
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth) ||
		(alphaRect.bottom <= 0) || (alphaRect.right <= 0)     ||
		(alphaRect.top >= alphaSurfaceHeight) ||
		(alphaRect.left >= alphaSurfaceWidth)
		)
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the dest rectangle
		// will fit within the destination surface area.
		if (destRect.left < 0 ||
			destRect.top < 0 ||
			destRect.right > destSurfaceWidth ||
			destRect.bottom > destSurfaceHeight)
		{
			return MGLERR_INVALIDRECT;
		}
	}
	else
	{
		RECT clippedDestRect;
	
		// OPTIMIZATION: Ensure that there's actually something to draw. If there's
		// not, we'll just waste time calling into the ARM code. - ptm
		IntersectRect(&clippedDestRect, &destRect, &clipper);
		
		// Only draw if there's something to draw
		if (IsRectEmpty(&clippedDestRect))
			return MGL_OK;
	}

	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// Get the alpha surface buffer
	hr = pAlphaSurface->GetBuffer(&alphaBuffDesc);
	if (FAILED(hr))
	{
		// Release source surface buffer
		pSrcSurface->ReleaseBuffer();
		
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	LONG alphaXPitch = alphaBuffDesc.xPitch / sizeof(WORD);
	LONG alphaYPitch = alphaBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLSTRETCHALPHABLT_OPACITY)
	{
		BL_StretchAlphaBltOpacity565(destBuffDesc.pBuffer, srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer, destXPitch, destYPitch, destSurfaceWidth, destSurfaceHeight,
			&destRect, srcXPitch, srcYPitch, &srcRect,
			alphaXPitch, alphaYPitch, &alphaRect, &clipper, dwFlags,
			pStretchAlphaBltFx->dwOpacity);
	}
	else
	{
		BL_StretchAlphaBlt565(destBuffDesc.pBuffer, srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer, destXPitch, destYPitch, destSurfaceWidth, destSurfaceHeight,
			&destRect, srcXPitch, srcYPitch, &srcRect,
			alphaXPitch, alphaYPitch, &alphaRect, &clipper, dwFlags);
	}
	
	// -----------------------------------------
	
	// Release the surface buffers
	pAlphaSurface->ReleaseBuffer();
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::StretchAlphaBltAdditive(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT* pAlphaRect, DWORD dwFlags, MGLSTRETCHALPHABLTFX* pStretchAlphaBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc, alphaBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, alphaSurfaceWidth, alphaSurfaceHeight,
		destSurfaceWidth, destSurfaceHeight, srcRectWidth, srcRectHeight,
		alphaRectWidth, alphaRectHeight;
	RECT srcRect, alphaRect, destRect, clipper;

	// Verify source and alpha surface ptrs
	if (!pSrcSurface || !pAlphaSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify opacity flag
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) && !pStretchAlphaBltFx)
		return MGLERR_INVALIDPARAMS;
	
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) &&
		pStretchAlphaBltFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) &&
		pStretchAlphaBltFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLSTRETCHALPHABLT_OPACITY;
	}
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get alpha surface rect
	alphaSurfaceWidth = pAlphaSurface->GetWidth();
	alphaSurfaceHeight = pAlphaSurface->GetHeight();
	if (pAlphaRect) {
		alphaRect.left = pAlphaRect->left;
		alphaRect.top = pAlphaRect->top;
		alphaRect.right = pAlphaRect->right;
		alphaRect.bottom = pAlphaRect->bottom;
	} else {
		alphaRect.left = 0;
		alphaRect.top = 0;
		alphaRect.right = alphaSurfaceWidth;
		alphaRect.bottom = alphaSurfaceHeight;
	}
	
	// Get destination surface rect
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pDestRect) {
		destRect.left = pDestRect->left;
		destRect.top = pDestRect->top;
		destRect.right = pDestRect->right;
		destRect.bottom = pDestRect->bottom;
	} else {
		destRect.left = 0;
		destRect.top = 0;
		destRect.right = destSurfaceWidth;
		destRect.bottom = destSurfaceHeight;
	}
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of alpha rectangle. */
	if (((alphaRect.bottom > alphaSurfaceHeight) || (alphaRect.bottom < 0) ||
		(alphaRect.top     > alphaSurfaceHeight) || (alphaRect.top    < 0) ||
		(alphaRect.left    > alphaSurfaceWidth)  || (alphaRect.left   < 0) ||
		(alphaRect.right   > alphaSurfaceWidth)  || (alphaRect.right  < 0) ||
		(alphaRect.right   <= alphaRect.left)               || (alphaRect.bottom <= alphaRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of destination rectangle. */
	if ((destRect.right <= destRect.left) || (destRect.bottom <= destRect.top))
	{
		return MGLERR_INVALIDRECT;
	}
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	// We want to be able to use the source X & Y positions
	// when reading the mask.  If the alpha mask rectangle
	// is larger than the source rectangle, shrink it so that we
	// don't read outside of the source image. - ptm
	if (alphaRectWidth > srcRectWidth ||
		alphaRectHeight > srcRectHeight)
	{
		if (alphaRectWidth > srcRectWidth)
			alphaRect.right -= alphaRectWidth-srcRectWidth;
			
		if (alphaRectHeight > srcRectHeight)
			alphaRect.bottom -= alphaRectHeight-srcRectHeight;
	}
	// If the alpha mask rectangle is smaller than the source rectangle,
	// shrink the source rectangle so that we don't read outside of
	// the alpha mask.
	else if (alphaRectWidth < srcRectWidth ||
		alphaRectHeight < srcRectHeight)
	{
		if (alphaRectWidth < srcRectWidth)
			srcRect.right -= srcRectWidth-alphaRectWidth;
			
		if (alphaRectHeight < srcRectHeight)
			srcRect.bottom -= srcRectHeight-alphaRectHeight;
	}
		
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destRect.left   >= destSurfaceWidth) ||
		(destRect.top    >= destSurfaceHeight) ||
		(destRect.bottom <= 0) || (destRect.right <= 0) ||	// added for StretchBlt()
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth) ||
		(alphaRect.bottom <= 0) || (alphaRect.right <= 0)     ||
		(alphaRect.top >= alphaSurfaceHeight) ||
		(alphaRect.left >= alphaSurfaceWidth)
		)
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the dest rectangle
		// will fit within the destination surface area.
		if (destRect.left < 0 ||
			destRect.top < 0 ||
			destRect.right > destSurfaceWidth ||
			destRect.bottom > destSurfaceHeight)
		{
			return MGLERR_INVALIDRECT;
		}
	}
	else
	{
		RECT clippedDestRect;
	
		// OPTIMIZATION: Ensure that there's actually something to draw. If there's
		// not, we'll just waste time calling into the ARM code. - ptm
		IntersectRect(&clippedDestRect, &destRect, &clipper);
		
		// Only draw if there's something to draw
		if (IsRectEmpty(&clippedDestRect))
			return MGL_OK;
	}
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// Get the alpha surface buffer
	hr = pAlphaSurface->GetBuffer(&alphaBuffDesc);
	if (FAILED(hr))
	{
		// Release source surface buffer
		pSrcSurface->ReleaseBuffer();
		
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	LONG alphaXPitch = alphaBuffDesc.xPitch / sizeof(WORD);
	LONG alphaYPitch = alphaBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLSTRETCHALPHABLT_OPACITY)
	{
		BL_StretchAlphaBltAdditiveOpacity565(destBuffDesc.pBuffer, srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer, destXPitch, destYPitch, destSurfaceWidth, destSurfaceHeight,
			&destRect, srcXPitch, srcYPitch, &srcRect,
			alphaXPitch, alphaYPitch, &alphaRect, &clipper, dwFlags,
			pStretchAlphaBltFx->dwOpacity);
	}
	else
	{
		BL_StretchAlphaBltAdditive565(destBuffDesc.pBuffer, srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer, destXPitch, destYPitch, destSurfaceWidth, destSurfaceHeight,
			&destRect, srcXPitch, srcYPitch, &srcRect,
			alphaXPitch, alphaYPitch, &alphaRect, &clipper, dwFlags);
	}
	
	// -----------------------------------------
	
	// Release the surface buffers
	pAlphaSurface->ReleaseBuffer();
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::StretchAlphaBltSubtractive(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT* pAlphaRect, DWORD dwFlags, MGLSTRETCHALPHABLTFX* pStretchAlphaBltFx)
{
	MGLBUFFDESC destBuffDesc, srcBuffDesc, alphaBuffDesc;
	HRESULT hr;
	LONG srcSurfaceWidth, srcSurfaceHeight, alphaSurfaceWidth, alphaSurfaceHeight,
		destSurfaceWidth, destSurfaceHeight, srcRectWidth, srcRectHeight,
		alphaRectWidth, alphaRectHeight;
	RECT srcRect, alphaRect, destRect, clipper;

	// Verify source and alpha surface ptrs
	if (!pSrcSurface || !pAlphaSurface) return MGLERR_INVALIDPARAMS;
	
	// Verify opacity flag
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) && !pStretchAlphaBltFx)
		return MGLERR_INVALIDPARAMS;
	
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) &&
		pStretchAlphaBltFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLSTRETCHALPHABLT_OPACITY) &&
		pStretchAlphaBltFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLSTRETCHALPHABLT_OPACITY;
	}
	
	// Get source surface rect
	srcSurfaceWidth = pSrcSurface->GetWidth();
	srcSurfaceHeight = pSrcSurface->GetHeight();
	if (pSrcRect) {
		srcRect.left = pSrcRect->left;
		srcRect.top = pSrcRect->top;
		srcRect.right = pSrcRect->right;
		srcRect.bottom = pSrcRect->bottom;
	} else {
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcSurfaceWidth;
		srcRect.bottom = srcSurfaceHeight;
	}
	
	// Get alpha surface rect
	alphaSurfaceWidth = pAlphaSurface->GetWidth();
	alphaSurfaceHeight = pAlphaSurface->GetHeight();
	if (pAlphaRect) {
		alphaRect.left = pAlphaRect->left;
		alphaRect.top = pAlphaRect->top;
		alphaRect.right = pAlphaRect->right;
		alphaRect.bottom = pAlphaRect->bottom;
	} else {
		alphaRect.left = 0;
		alphaRect.top = 0;
		alphaRect.right = alphaSurfaceWidth;
		alphaRect.bottom = alphaSurfaceHeight;
	}
	
	// Get destination surface rect
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pDestRect) {
		destRect.left = pDestRect->left;
		destRect.top = pDestRect->top;
		destRect.right = pDestRect->right;
		destRect.bottom = pDestRect->bottom;
	} else {
		destRect.left = 0;
		destRect.top = 0;
		destRect.right = destSurfaceWidth;
		destRect.bottom = destSurfaceHeight;
	}
	
	/* First check for the validity of source rectangle. */
	if (((srcRect.bottom > srcSurfaceHeight) || (srcRect.bottom < 0) ||
		(srcRect.top     > srcSurfaceHeight) || (srcRect.top    < 0) ||
		(srcRect.left    > srcSurfaceWidth)  || (srcRect.left   < 0) ||
		(srcRect.right   > srcSurfaceWidth)  || (srcRect.right  < 0) ||
		(srcRect.right   <= srcRect.left)               || (srcRect.bottom <= srcRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of alpha rectangle. */
	if (((alphaRect.bottom > alphaSurfaceHeight) || (alphaRect.bottom < 0) ||
		(alphaRect.top     > alphaSurfaceHeight) || (alphaRect.top    < 0) ||
		(alphaRect.left    > alphaSurfaceWidth)  || (alphaRect.left   < 0) ||
		(alphaRect.right   > alphaSurfaceWidth)  || (alphaRect.right  < 0) ||
		(alphaRect.right   <= alphaRect.left)               || (alphaRect.bottom <= alphaRect.top)))
	{
		return MGLERR_INVALIDRECT;
	}
	
	/* First check for the validity of destination rectangle. */
	if ((destRect.right <= destRect.left) || (destRect.bottom <= destRect.top))
	{
		return MGLERR_INVALIDRECT;
	}
	
	srcRectWidth = srcRect.right - srcRect.left;
	srcRectHeight = srcRect.bottom - srcRect.top;
	
	alphaRectWidth = alphaRect.right - alphaRect.left;
	alphaRectHeight = alphaRect.bottom - alphaRect.top;
	
	// We want to be able to use the source X & Y positions
	// when reading the mask.  If the alpha mask rectangle
	// is larger than the source rectangle, shrink it so that we
	// don't read outside of the source image. - ptm
	if (alphaRectWidth > srcRectWidth ||
		alphaRectHeight > srcRectHeight)
	{
		if (alphaRectWidth > srcRectWidth)
			alphaRect.right -= alphaRectWidth-srcRectWidth;
			
		if (alphaRectHeight > srcRectHeight)
			alphaRect.bottom -= alphaRectHeight-srcRectHeight;
	}
	// If the alpha mask rectangle is smaller than the source rectangle,
	// shrink the source rectangle so that we don't read outside of
	// the alpha mask.
	else if (alphaRectWidth < srcRectWidth ||
		alphaRectHeight < srcRectHeight)
	{
		if (alphaRectWidth < srcRectWidth)
			srcRect.right -= srcRectWidth-alphaRectWidth;
			
		if (alphaRectHeight < srcRectHeight)
			srcRect.bottom -= srcRectHeight-alphaRectHeight;
	}
		
	/* Now handle negative values in the rectangles.
	    Handle the case where nothing is to be done.
	*/
	if ((destRect.left   >= destSurfaceWidth) ||
		(destRect.top    >= destSurfaceHeight) ||
		(destRect.bottom <= 0) || (destRect.right <= 0) ||	// added for StretchBlt()
		(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
		(srcRect.top >= srcSurfaceHeight) ||
		(srcRect.left >= srcSurfaceWidth) ||
		(alphaRect.bottom <= 0) || (alphaRect.right <= 0)     ||
		(alphaRect.top >= alphaSurfaceHeight) ||
		(alphaRect.left >= alphaSurfaceWidth)
		)
	{
		return MGL_OK;
	}
	
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Clipping is disabled when the clipper rect is {0, 0, 0, 0}
	// If that is the case, verify blit coordinates.
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the dest rectangle
		// will fit within the destination surface area.
		if (destRect.left < 0 ||
			destRect.top < 0 ||
			destRect.right > destSurfaceWidth ||
			destRect.bottom > destSurfaceHeight)
		{
			return MGLERR_INVALIDRECT;
		}
	}
	else
	{
		RECT clippedDestRect;
	
		// OPTIMIZATION: Ensure that there's actually something to draw. If there's
		// not, we'll just waste time calling into the ARM code. - ptm
		IntersectRect(&clippedDestRect, &destRect, &clipper);
		
		// Only draw if there's something to draw
		if (IsRectEmpty(&clippedDestRect))
			return MGL_OK;
	}
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the source surface buffer
	hr = pSrcSurface->GetBuffer(&srcBuffDesc);
	if (FAILED(hr))
	{
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// Get the alpha surface buffer
	hr = pAlphaSurface->GetBuffer(&alphaBuffDesc);
	if (FAILED(hr))
	{
		// Release source surface buffer
		pSrcSurface->ReleaseBuffer();
		
		// Release destination surface buffer
		ReleaseBuffer();
		return hr;
	}
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG srcXPitch = srcBuffDesc.xPitch / sizeof(WORD);
	LONG srcYPitch = srcBuffDesc.yPitch / sizeof(WORD);
	LONG alphaXPitch = alphaBuffDesc.xPitch / sizeof(WORD);
	LONG alphaYPitch = alphaBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLSTRETCHALPHABLT_OPACITY)
	{
		BL_StretchAlphaBltSubtractiveOpacity565(destBuffDesc.pBuffer, srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer, destXPitch, destYPitch, destSurfaceWidth, destSurfaceHeight,
			&destRect, srcXPitch, srcYPitch, &srcRect,
			alphaXPitch, alphaYPitch, &alphaRect, &clipper, dwFlags,
			pStretchAlphaBltFx->dwOpacity);
	}
	else
	{
		BL_StretchAlphaBltSubtractive565(destBuffDesc.pBuffer, srcBuffDesc.pBuffer,
			alphaBuffDesc.pBuffer, destXPitch, destYPitch, destSurfaceWidth, destSurfaceHeight,
			&destRect, srcXPitch, srcYPitch, &srcRect,
			alphaXPitch, alphaYPitch, &alphaRect, &clipper, dwFlags);
	}
	
	// -----------------------------------------
	
	// Release the surface buffers
	pAlphaSurface->ReleaseBuffer();
	pSrcSurface->ReleaseBuffer();
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::GetPixel(LONG x, LONG y, COLORREF* pColor)
{
	MGLBUFFDESC buffDesc;
	HRESULT hr;

	// Verify parameters
	if (!pColor) return MGLERR_INVALIDPARAMS;
	
	// Verify coordinates
	if (x < 0 || y < 0 || x >= (LONG)GetWidth() || y >= (LONG)GetHeight())
		return MGLERR_INVALIDRECT;
	
	// ---------------------------------------------
	
	// Get the surface buffer
	hr = GetBuffer(&buffDesc);
	if (FAILED(hr)) return hr;
	
	// Get the pixel
	switch (buffDesc.dwPixelFormat)
	{
		case MGLPIXELFORMAT_555:
			{
			LONG xPitch = buffDesc.xPitch / sizeof(WORD);
			LONG yPitch = buffDesc.yPitch / sizeof(WORD);
			
			*pColor = BL_GetPixel555(buffDesc.pBuffer, xPitch, yPitch, x, y);
			}
			break;
			
		case MGLPIXELFORMAT_565:
			{
			LONG xPitch = buffDesc.xPitch / sizeof(WORD);
			LONG yPitch = buffDesc.yPitch / sizeof(WORD);
			
			*pColor = BL_GetPixel565(buffDesc.pBuffer, xPitch, yPitch, x, y);
			}
			break;
			
		default: // Unsupported pixel format
			hr = MGLERR_UNSUPPORTEDMODE;
			goto error;
	}
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();
	
	return MGL_OK;
	
	error:
	
	// Release the surface buffer
	ReleaseBuffer();
	
	return hr;
}

// Sets a single pixel. This function respects the defined clipping rectangle
// and will not set the pixel if coordinates outside of the clipping rectangle
// are specified.
HRESULT CMglSurface::SetPixel(LONG x, LONG y, COLORREF dwColor)
{
	MGLBUFFDESC destBuffDesc;
	RECT clipper;
	HRESULT hr;
	
	// Verify coordinates
	if (x < 0 || y < 0 || x >= (LONG)GetWidth() || y >= (LONG)GetHeight())
		return MGLERR_INVALIDRECT;
		
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// If coordinates specified are outside of the clipping
	// rectangle, don't set the pixel.
	if (x < clipper.left || y < clipper.top ||
		x >= clipper.right || y >= clipper.bottom)
		return MGL_OK;
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Set the pixel
	switch (destBuffDesc.dwPixelFormat)
	{
		case MGLPIXELFORMAT_555:
			{
			LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
			LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
			
			BL_SetPixel555(destBuffDesc.pBuffer, destXPitch, destYPitch, x, y, dwColor);
			}
			break;
			
		case MGLPIXELFORMAT_565:
			{
			LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
			LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
			
			BL_SetPixel565(destBuffDesc.pBuffer, destXPitch, destYPitch, x, y, dwColor);
			}
			break;
			
		default: // Unsupported pixel format
			hr = MGLERR_UNSUPPORTEDMODE;
			goto error;
	}
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();
	
	return MGL_OK;
	
	error:
	
	// Release the surface buffer
	ReleaseBuffer();
	
	return hr;
}

HRESULT CMglSurface::SetPixels(MGLPIXEL *pPixelArray, DWORD dwElementSize, DWORD dwElementCount, DWORD dwFlags)
{
	MGLBUFFDESC destBuffDesc;
	HRESULT hr;
	LONG destWidth, destHeight;
	BYTE *pb, *pbEnd;
	
	// Verify parameters
	if (!pPixelArray || dwElementSize == 0 || dwElementCount == 0)
		return MGLERR_INVALIDPARAMS;
	
	// ---------------------------------------------
	
	// Get destination surface dimensions
	destWidth = (LONG)GetWidth();
	destHeight = (LONG)GetHeight();
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Only RGB555 and RGB565 pixel formats are currently supported
	if (destBuffDesc.dwPixelFormat != MGLPIXELFORMAT_555 &&
		destBuffDesc.dwPixelFormat != MGLPIXELFORMAT_565)
	{
		// Release the surface buffer
		ReleaseBuffer();
	
		return MGLERR_UNSUPPORTEDMODE;
	}
	
	// Compute 16-bit pitches
	LONG destXPitch16 = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch16 = destBuffDesc.yPitch / sizeof(WORD);
	
	// Get a pointer to the start and end of the pixel array
	pb = (BYTE*)pPixelArray;
	pbEnd = (BYTE*)pPixelArray+(dwElementSize*dwElementCount);
	
	// Iterate through each pixel
	while (pb < pbEnd)
	{
		// Get a pointer to the current pixel
		MGLPIXEL *pPixel = (MGLPIXEL*)pb;
		LONG x, y;
		BOOL bUseOpacity;
		
		// Don't draw the pixel if it's disabled or if it's not visible
		if ((pPixel->dwFlags & MGLPIXEL_DISABLED) ||
			((pPixel->dwFlags & MGLPIXEL_OPACITY) &&
			pPixel->pixelfx.dwOpacity == OPACITY_0))
		{
			// Advance to the next pixel in the array
			pb += dwElementSize;
			
			continue;
		}
		
		// Get the coordinates from the pixel
		x = pPixel->x;
		y = pPixel->y;
		
		// Convert from fixed point 16.16, if needed
		if (dwFlags & MGLSETPIXELS_FIXEDPOINT)
		{
			x = x >> 16;
			y = y >> 16;
		}
		
		// Verify pixel coordinates if coordinate checking is not disabled
		if (!(dwFlags & MGLSETPIXELS_NOCOORDCHECK))
		{
			// Handle the case where the coordinates are invalid
			if (x < 0 || y < 0 || x >= destWidth || y >= destHeight)
			{
				// Advance to the next pixel in the array
				pb += dwElementSize;
				
				continue;
			}
		}
		
		// Determine whether or not to use opacity
		bUseOpacity = ((pPixel->dwFlags & MGLPIXEL_OPACITY) &&
						pPixel->pixelfx.dwOpacity < OPACITY_100);
	
		// Set the pixel
		switch (destBuffDesc.dwPixelFormat)
		{
			case MGLPIXELFORMAT_555:
				{
					if (bUseOpacity)
					{
						BL_SetPixelOpacity555(destBuffDesc.pBuffer, destXPitch16,
							destYPitch16, x, y, pPixel->dwColor, pPixel->pixelfx.dwOpacity);
					}
					else
					{
						BL_SetPixel555(destBuffDesc.pBuffer, destXPitch16,
							destYPitch16, x, y, pPixel->dwColor);
					}	
				}
				break;
				
			case MGLPIXELFORMAT_565:
				{
					if (bUseOpacity)
					{
						BL_SetPixelOpacity565(destBuffDesc.pBuffer, destXPitch16,
							destYPitch16, x, y, pPixel->dwColor, pPixel->pixelfx.dwOpacity);
					}
					else
					{
						BL_SetPixel565(destBuffDesc.pBuffer, destXPitch16,
							destYPitch16, x, y, pPixel->dwColor);
					}
				}
				break;
				
			default:
				break;
		}
		
		// Advance to the next pixel in the array
		pb += dwElementSize;
	}
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::SetPixels(MGLPIXELNODE *pHead, DWORD dwFlags)
{
	MGLBUFFDESC destBuffDesc;
	HRESULT hr;
	LONG destWidth, destHeight;
	MGLPIXELNODE *pPixelNode;
	
	// Verify parameters
	if (!pHead)
		return MGLERR_INVALIDPARAMS;
	
	// ---------------------------------------------
	
	// Get destination surface dimensions
	destWidth = (LONG)GetWidth();
	destHeight = (LONG)GetHeight();
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Only RGB555 and RGB565 pixel formats are currently supported
	if (destBuffDesc.dwPixelFormat != MGLPIXELFORMAT_555 &&
		destBuffDesc.dwPixelFormat != MGLPIXELFORMAT_565)
	{
		// Release the surface buffer
		ReleaseBuffer();
	
		return MGLERR_UNSUPPORTEDMODE;
	}
	
	// Compute 16-bit pitches
	LONG destXPitch16 = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch16 = destBuffDesc.yPitch / sizeof(WORD);
	
	// Iterate through each pixel
	pPixelNode = pHead;
	while (pPixelNode)
	{
		// Get a pointer to the current pixel
		MGLPIXEL *pPixel = &pPixelNode->pixel;
		LONG x, y;
		BOOL bUseOpacity;
		
		// Don't draw the pixel if it's disabled or if it's not visible
		if ((pPixel->dwFlags & MGLPIXEL_DISABLED) ||
			((pPixel->dwFlags & MGLPIXEL_OPACITY) &&
			pPixel->pixelfx.dwOpacity == OPACITY_0))
		{
			// Advance to the next pixel in the list
			pPixelNode = pPixelNode->pNext;
			
			continue;
		}
		
		// Get the coordinates from the pixel
		x = pPixel->x;
		y = pPixel->y;
		
		// Convert from fixed point 16.16, if needed
		if (dwFlags & MGLSETPIXELS_FIXEDPOINT)
		{
			x = x >> 16;
			y = y >> 16;
		}
		
		// Verify pixel coordinates if coordinate checking is not disabled
		if (!(dwFlags & MGLSETPIXELS_NOCOORDCHECK))
		{
			// Handle the case where the coordinates are invalid
			if (x < 0 || y < 0 || x >= destWidth || y >= destHeight)
			{
				// Advance to the next pixel in the list
				pPixelNode = pPixelNode->pNext;
				
				continue;
			}
		}
		
		// Determine whether or not to use opacity
		bUseOpacity = ((pPixel->dwFlags & MGLPIXEL_OPACITY) &&
						pPixel->pixelfx.dwOpacity < OPACITY_100);
	
		// Set the pixel
		switch (destBuffDesc.dwPixelFormat)
		{
			case MGLPIXELFORMAT_555:
				{
					if (bUseOpacity)
					{
						BL_SetPixelOpacity555(destBuffDesc.pBuffer, destXPitch16,
							destYPitch16, x, y, pPixel->dwColor, pPixel->pixelfx.dwOpacity);
					}
					else
					{
						BL_SetPixel555(destBuffDesc.pBuffer, destXPitch16,
							destYPitch16, x, y, pPixel->dwColor);
					}	
				}
				break;
				
			case MGLPIXELFORMAT_565:
				{
					if (bUseOpacity)
					{
						BL_SetPixelOpacity565(destBuffDesc.pBuffer, destXPitch16,
							destYPitch16, x, y, pPixel->dwColor, pPixel->pixelfx.dwOpacity);
					}
					else
					{
						BL_SetPixel565(destBuffDesc.pBuffer, destXPitch16,
							destYPitch16, x, y, pPixel->dwColor);
					}
				}
				break;
				
			default:
				break;
		}
		
		// Advance to the next pixel in the list
		pPixelNode = pPixelNode->pNext;
	}
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();
	
	return MGL_OK;
}

HRESULT CMglSurface::DrawLine(LONG x1, LONG y1, LONG x2, LONG y2, COLORREF dwColor, DWORD dwFlags, MGLDRAWLINEFX* pDrawLineFx)
{
	MGLBUFFDESC destBuffDesc;
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT destRect, clipper;
	POINT pt1, pt2;
	
	// Verify opacity flag
	if ((dwFlags & MGLDRAWLINE_OPACITY) && !pDrawLineFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if opacity is zero
	if ((dwFlags & MGLDRAWLINE_OPACITY) && pDrawLineFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear opacity flag if line is fully opaque
	if ((dwFlags & MGLDRAWLINE_OPACITY) && pDrawLineFx->dwOpacity == OPACITY_100)
		dwFlags &= ~MGLDRAWLINE_OPACITY;
		
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Get destination surface width & height
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	pt1.x = x1;
	pt1.y = y1;
	pt2.x = x2;
	pt2.y = y2;
	
	// Validate draw coordinates
	if (IsRectEmpty(&clipper))
	{
		destRect.left = 0;
		destRect.top = 0;
		destRect.right = destSurfaceWidth;
		destRect.bottom = destSurfaceHeight;
	
		// Clipping is not enabled. Ensure the line
		// will fit within the destination surface area.
		if (!PtInRect(&destRect, pt1) ||
			!PtInRect(&destRect, pt2))
		{
			return MGLERR_INVALIDRECT;
		}
	}
	else
	{
		// Clip the line coordinates
		if (!DrawLineHelper::ClipLine(pt1, pt2, clipper))
			return MGL_OK;
	}
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLDRAWLINE_ANTIALIAS)
	{
		if (dwFlags & MGLDRAWLINE_OPACITY)
		{
			// Draw the antialiased line with opacity
			BL_DrawWuLineOpacity565(destBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				destSurfaceWidth,
				destSurfaceHeight,
				pt1.x,
				pt1.y,
				pt2.x,
				pt2.y,
				dwColor,
				pDrawLineFx->dwOpacity);
		}
		else
		{
			// Draw the antialiased line
			BL_DrawWuLine565(destBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				destSurfaceWidth,
				destSurfaceHeight,
				pt1.x,
				pt1.y,
				pt2.x,
				pt2.y,
				dwColor);
		}
	}
	else
	{
		if (dwFlags & MGLDRAWLINE_OPACITY)
		{
			// Draw the line with opacity
			BL_DrawLineOpacity565(destBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				destSurfaceWidth,
				destSurfaceHeight,
				pt1.x,
				pt1.y,
				pt2.x,
				pt2.y,
				dwColor,
				pDrawLineFx->dwOpacity);
		}
		else
		{
			// Draw the line
			BL_DrawLine565(destBuffDesc.pBuffer,
				destXPitch,
				destYPitch,
				destSurfaceWidth,
				destSurfaceHeight,
				pt1.x,
				pt1.y,
				pt2.x,
				pt2.y,
				dwColor);
		}
	}
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();
	
	return MGL_OK;		
}		

// Draws a rectangle.
HRESULT CMglSurface::DrawRect(RECT* pRect, COLORREF dwColor, DWORD dwFlags, MGLDRAWRECTFX *pDrawRectFx)
{
	MGLBUFFDESC destBuffDesc;
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT rect, clipper;

	// Verify opacity flag
	if ((dwFlags & MGLDRAWRECT_OPACITY) && !pDrawRectFx)
		return MGLERR_INVALIDPARAMS;

	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLDRAWRECT_OPACITY) &&
		pDrawRectFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLDRAWRECT_OPACITY) &&
		pDrawRectFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLDRAWRECT_OPACITY;
	}
		
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
		
	// Get draw rectangle
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pRect) {
		rect.left = pRect->left;
		rect.top = pRect->top;
		rect.right = pRect->right;
		rect.bottom = pRect->bottom;
	} else {
		rect.left = 0;
		rect.top = 0;
		rect.right = destSurfaceWidth;
		rect.bottom = destSurfaceHeight;
	}
	
	// Validate fill rectangle
	if (IsRectEmpty(&rect))
		return MGLERR_INVALIDRECT;
		
	// The following is just a quickie optimization...
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the draw rectangle
		// will fit within the destination surface area.
		if (rect.left < 0 ||
			rect.top < 0 ||
			rect.right > destSurfaceWidth ||
			rect.bottom > destSurfaceHeight)
		return MGLERR_INVALIDRECT;
	}
	
	// ---------------------------------------------
	// TW Video Surface Blitting
	// ---------------------------------------------
	
	// If surface is located in video memory,
	// use TwGfx hardware blitter, when possible.
	if (!(dwFlags & MGLFILLRECT_OPACITY) &&
		(m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY))
	{
		BOOL bDestVideoLocked;
		TwGfxRectType twRect;
		TwGfxPackedRGBType twColor;
			
		bDestVideoLocked = (m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED);

		// If surface was video locked, unlock it.
		if (bDestVideoLocked)	UnlockVideoSurface();

		twRect.x = rect.left;
		twRect.y = rect.top;
		twRect.w = rect.right - rect.left;
		twRect.h = rect.bottom - rect.top;
		
		twColor = TwGfxComponentsToPackedRGB(
			GetRValue(dwColor),
			GetGValue(dwColor),
			GetBValue(dwColor)
			);
		
		TwGfxDrawRect((TwGfxSurfaceHandle)m_surface.pBuffer,
	                  &twRect, twColor);
		
		// If surface was video locked, relock it.
		if (bDestVideoLocked)	LockVideoSurface();
		
		// We're done!
		return MGL_OK;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG rectX2 = rect.right-1;
	LONG rectY2 = rect.bottom-1;
	
	if (dwFlags & MGLDRAWRECT_OPACITY)
	{
		BL_DrawRectOpacity565(destBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			destSurfaceWidth,
			destSurfaceHeight,
			&clipper,
			rect.left,
			rect.top,
			rectX2,
			rectY2,
			dwColor,
			pDrawRectFx->dwOpacity);
	}
	else
	{
		BL_DrawRect565(destBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			destSurfaceWidth,
			destSurfaceHeight,
			&clipper,
			rect.left,
			rect.top,
			rectX2,
			rectY2,
			dwColor);
	}
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::FillRect(RECT* pRect, COLORREF dwColor, DWORD dwFlags, MGLFILLRECTFX* pFillRectFx)
{
	MGLBUFFDESC destBuffDesc;
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT fillRect, clipper;
	DWORD dwNativeColor;

	// Verify opacity flag
	if ((dwFlags & MGLFILLRECT_OPACITY) && !pFillRectFx)
		return MGLERR_INVALIDPARAMS;

	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLFILLRECT_OPACITY) &&
		pFillRectFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLFILLRECT_OPACITY) &&
		pFillRectFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLFILLRECT_OPACITY;
	}
		
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
		
	// Get fill rectangle
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pRect) {
		fillRect.left = pRect->left;
		fillRect.top = pRect->top;
		fillRect.right = pRect->right;
		fillRect.bottom = pRect->bottom;
	} else {
		fillRect.left = 0;
		fillRect.top = 0;
		fillRect.right = destSurfaceWidth;
		fillRect.bottom = destSurfaceHeight;
	}
	
	// Validate fill rectangle
	if (IsRectEmpty(&fillRect))
		return MGLERR_INVALIDRECT;
		
	// If clipping is enabled, clip the fill rectangle
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the fill rectangle
		// will fit within the destination surface area.
		if (fillRect.left < 0 ||
			fillRect.top < 0 ||
			fillRect.right > destSurfaceWidth ||
			fillRect.bottom > destSurfaceHeight)
		return MGLERR_INVALIDRECT;
	}
	else
	{
		// Clip the fill rectangle
		IntersectRect(&fillRect, &fillRect, &clipper);
		
		// If there's nothing to draw, then we're done.
		if (IsRectEmpty(&fillRect))
			return MGL_OK;
	}
	
	// ---------------------------------------------
	// TW Video Surface Blitting
	// ---------------------------------------------
	
	// If surface is located in video memory,
	// use TwGfx hardware blitter, when possible.
	if (!(dwFlags & MGLFILLRECT_OPACITY) &&
		(m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY))
	{
		BOOL bDestVideoLocked;
		TwGfxRectType twFillRect;
		TwGfxPackedRGBType twFillColor;
			
		bDestVideoLocked = (m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED);

		// If surface was video locked, unlock it.
		if (bDestVideoLocked)	UnlockVideoSurface();

		twFillRect.x = fillRect.left;
		twFillRect.y = fillRect.top;
		twFillRect.w = fillRect.right - fillRect.left;
		twFillRect.h = fillRect.bottom - fillRect.top;
		
		twFillColor = TwGfxComponentsToPackedRGB(
			GetRValue(dwColor),
			GetGValue(dwColor),
			GetBValue(dwColor)
			);	
		
		TwGfxFillRect((TwGfxSurfaceHandle)m_surface.pBuffer,
	                  &twFillRect, twFillColor);
		
		// If surface was video locked, relock it.
		if (bDestVideoLocked)	LockVideoSurface();
		
		// We're done!
		return MGL_OK;
	}
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	LONG fillWidth = fillRect.right-fillRect.left;
	LONG fillHeight = fillRect.bottom-fillRect.top;
	
	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	if (dwFlags & MGLFILLRECT_OPACITY)
	{
		BL_FillRectOpacity565(destBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			destSurfaceWidth,
			destSurfaceHeight,
			fillRect.left,
			fillRect.top,
			fillWidth,
			fillHeight,
			dwNativeColor,
			pFillRectFx->dwOpacity);
	}
	else
	{
		BL_FillRect565(destBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			destSurfaceWidth,
			destSurfaceHeight,
			fillRect.left,
			fillRect.top,
			fillWidth,
			fillHeight,
			dwNativeColor);
	}
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::FillGradient(RECT* pRect, COLORREF dwColor1, DWORD dwColor2, DWORD dwFlags, MGLFILLGRADIENTFX* pFillGradientFx)
{
	MGLBUFFDESC destBuffDesc;
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT fillRect, clipper;
	DWORD dwOpacity;
	
	// Verify opacity flag
	if ((dwFlags & MGLFILLGRADIENT_OPACITY) && !pFillGradientFx)
		return MGLERR_INVALIDPARAMS;

	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLFILLGRADIENT_OPACITY) &&
		pFillGradientFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLFILLGRADIENT_OPACITY) &&
		pFillGradientFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLFILLRECT_OPACITY;
	}
		
	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	// Get fill rectangle
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	if (pRect) {
		fillRect.left = pRect->left;
		fillRect.top = pRect->top;
		fillRect.right = pRect->right;
		fillRect.bottom = pRect->bottom;
	} else {
		fillRect.left = 0;
		fillRect.top = 0;
		fillRect.right = destSurfaceWidth;
		fillRect.bottom = destSurfaceHeight;
	}
	
	// Validate fill rectangle
	if (IsRectEmpty(&fillRect))
		return MGLERR_INVALIDRECT;
	
	// ---------------------------------------------
	// TW Video Surface Blitting
	// ---------------------------------------------
	
	// If surface is located in video memory,
	// use TwGfx hardware blitter, when possible.
	if (!(dwFlags & (MGLFILLGRADIENT_OPACITY | MGLFILLGRADIENT_DITHER)) &&
		(m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY))
	{
		BOOL bDestVideoLocked;
			
		bDestVideoLocked = (m_surface.dwFlags & MGLSURFACE_VIDEOLOCKED);

		// If surface was video locked, unlock it.
		if (bDestVideoLocked)	UnlockVideoSurface();

		BL_FillGradient_TwGfx((TwGfxSurfaceHandle)m_surface.pBuffer,
			fillRect.left,
			fillRect.top,
			fillRect.right,
			fillRect.bottom,
			dwColor1,
			dwColor2,
			dwFlags);
		
		// If surface was video locked, relock it.
		if (bDestVideoLocked)	LockVideoSurface();
		
		// We're done!
		return MGL_OK;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	// Create opacity value
	if (dwFlags & MGLFILLGRADIENT_OPACITY)
		dwOpacity = pFillGradientFx->dwOpacity;
	else
		dwOpacity = 0;	// unused
	
	if (dwFlags & MGLFILLGRADIENT_DITHER)
	{
		BL_FillGradientDither565(destBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			destSurfaceWidth,
			destSurfaceHeight,
			&clipper,
			fillRect.left,
			fillRect.top,
			fillRect.right,
			fillRect.bottom,
			dwColor1,
			dwColor2,
			dwFlags,
			dwOpacity);
	}
	else
	{
		BL_FillGradient565(destBuffDesc.pBuffer,
			destXPitch,
			destYPitch,
			destSurfaceWidth,
			destSurfaceHeight,
			&clipper,
			fillRect.left,
			fillRect.top,
			fillRect.right,
			fillRect.bottom,
			dwColor1,
			dwColor2,
			dwFlags,
			dwOpacity);
	}

	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::DrawEllipse(RECT *pRect, COLORREF dwColor, DWORD dwFlags, MGLDRAWELLIPSEFX *pDrawEllipseFx)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT clipper, rect;
	MGLBUFFDESC destBuffDesc;
	
	// Verify parameters
	if ((dwFlags & MGLDRAWELLIPSE_OPACITY) && !pDrawEllipseFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLDRAWELLIPSE_OPACITY) &&
		pDrawEllipseFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLDRAWELLIPSE_OPACITY) &&
		pDrawEllipseFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLDRAWELLIPSE_OPACITY;
	}

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Get draw rectangle
	if (pRect) {
		rect.left = pRect->left;
		rect.top = pRect->top;
		rect.right = pRect->right;
		rect.bottom = pRect->bottom;
	} else {
		rect.left = 0;
		rect.top = 0;
		rect.right = destSurfaceWidth;
		rect.bottom = destSurfaceHeight;
	}
	
	// Validate fill rectangle
	if (IsRectEmpty(&rect))
		return MGLERR_INVALIDRECT;
		
	// If clipping is enabled, clip the fill rectangle
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the fill rectangle
		// will fit within the destination surface area.
		if (rect.left < 0 ||
			rect.top < 0 ||
			rect.right > destSurfaceWidth ||
			rect.bottom > destSurfaceHeight)
		return MGLERR_INVALIDRECT;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLDRAWELLIPSE_OPACITY)
	{
		BL_DrawEllipseOpacity565 (destBuffDesc.pBuffer, destXPitch, destYPitch,
			destSurfaceWidth, destSurfaceHeight, &clipper, &rect, dwColor,
			pDrawEllipseFx->dwOpacity);
	}
	else
	{
		BL_DrawEllipse565 (destBuffDesc.pBuffer, destXPitch, destYPitch,
			destSurfaceWidth, destSurfaceHeight, &clipper, &rect, dwColor);
	}

	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::FillEllipse(RECT *pRect, COLORREF dwColor, DWORD dwFlags, MGLFILLELLIPSEFX *pFillEllipseFx)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT clipper, rect;
	MGLBUFFDESC destBuffDesc;
	
	// Verify parameters
	if ((dwFlags & MGLFILLELLIPSE_OPACITY) && !pFillEllipseFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLFILLELLIPSE_OPACITY) &&
		pFillEllipseFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLFILLELLIPSE_OPACITY) &&
		pFillEllipseFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLFILLELLIPSE_OPACITY;
	}

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Get draw rectangle
	if (pRect) {
		rect.left = pRect->left;
		rect.top = pRect->top;
		rect.right = pRect->right;
		rect.bottom = pRect->bottom;
	} else {
		rect.left = 0;
		rect.top = 0;
		rect.right = destSurfaceWidth;
		rect.bottom = destSurfaceHeight;
	}
	
	// Validate fill rectangle
	if (IsRectEmpty(&rect))
		return MGLERR_INVALIDRECT;
		
	// If clipping is enabled, clip the fill rectangle
	if (IsRectEmpty(&clipper))
	{
		// Clipping is not enabled. Ensure the fill rectangle
		// will fit within the destination surface area.
		if (rect.left < 0 ||
			rect.top < 0 ||
			rect.right > destSurfaceWidth ||
			rect.bottom > destSurfaceHeight)
		return MGLERR_INVALIDRECT;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	if (dwFlags & MGLFILLELLIPSE_OPACITY)
	{
		BL_FillEllipseOpacity565 (destBuffDesc.pBuffer, destXPitch, destYPitch,
			destSurfaceWidth, destSurfaceHeight, &clipper, &rect, dwColor,
			pFillEllipseFx->dwOpacity);
	}
	else
	{
		BL_FillEllipse565 (destBuffDesc.pBuffer, destXPitch, destYPitch,
			destSurfaceWidth, destSurfaceHeight, &clipper, &rect, dwColor);
	}

	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::DrawPolyline(POINT *pPointArray, DWORD dwNumPoints, COLORREF dwColor, DWORD dwFlags, MGLDRAWPOLYLINEFX *pDrawPolylineFx)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT clipper;
	MGLBUFFDESC destBuffDesc;
	
	// Verify parameters
	if (!pPointArray || dwNumPoints == 0)
		return MGLERR_INVALIDPARAMS;

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Adjust clipper if no clip rectangle is defined
	if (IsRectEmpty(&clipper))
	{
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	BL_DrawPolyline565 (destBuffDesc.pBuffer, destXPitch, destYPitch,
		destSurfaceWidth, destSurfaceHeight, &clipper, dwColor,
		pPointArray, dwNumPoints, dwFlags);

	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::DrawPolygon(POINT *pPointArray, DWORD dwNumPoints, COLORREF dwColor, DWORD dwFlags, MGLDRAWPOLYGONFX *pDrawPolygonFx)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT clipper;
	MGLBUFFDESC destBuffDesc;
	
	// Verify parameters
	if (!pPointArray || dwNumPoints == 0)
		return MGLERR_INVALIDPARAMS;

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Adjust clipper if no clip rectangle is defined
	if (IsRectEmpty(&clipper))
	{
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	BL_DrawPolygon565 (destBuffDesc.pBuffer, destXPitch, destYPitch,
		destSurfaceWidth, destSurfaceHeight, &clipper, dwColor,
		pPointArray, dwNumPoints, dwFlags);

	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::FillPolygon(POINT *pPointArray, DWORD dwNumPoints, COLORREF dwColor, DWORD dwFlags, MGLFILLPOLYGONFX *pFillPolygonFx)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT clipper;
	MGLBUFFDESC destBuffDesc;
	DWORD dwOpacity = 0;
	
	// Verify parameters
	if (!pPointArray || dwNumPoints == 0)
		return MGLERR_INVALIDPARAMS;
		
	// Verify FX parameters
	if ((dwFlags & MGLFILLPOLYGON_OPACITY) && !pFillPolygonFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLFILLPOLYGON_OPACITY) &&
		pFillPolygonFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLFILLPOLYGON_OPACITY) &&
		pFillPolygonFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLFILLPOLYGON_OPACITY;
	}

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Adjust clipper if no clip rectangle is defined
	if (IsRectEmpty(&clipper))
	{
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	// Create opacity value
	if (dwFlags & MGLFILLPOLYGON_OPACITY)
		dwOpacity = pFillPolygonFx->dwOpacity;
	
	BL_FillPolygon565 (destBuffDesc.pBuffer, destXPitch, destYPitch,
		destSurfaceWidth, destSurfaceHeight, &clipper, dwColor,
		pPointArray, dwNumPoints, dwFlags, dwOpacity);

	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::DrawBezier(LONG startX, LONG startY,
			LONG c1x, LONG c1y,	/* control point 1 */
			LONG c2x, LONG c2y,	/* control point 2 */
			LONG endX, LONG endY, COLORREF dwColor, DWORD dwFlags, MGLDRAWBEZIER *pDrawBezierFx)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT clipper;
	MGLBUFFDESC destBuffDesc;
	POINT points[4];

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Adjust clipper if no clip rectangle is defined
	if (IsRectEmpty(&clipper))
	{
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}
	
	// Initialize the points array
	points[0].x = startX;
	points[0].y = startY;
	points[1].x = c1x;
	points[1].y = c1y;
	points[2].x = c2x;
	points[2].y = c2y;
	points[3].x = endX;
	points[3].y = endY;

	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	BL_DrawBezier565(destBuffDesc.pBuffer, destXPitch, destYPitch,
		destSurfaceWidth, destSurfaceHeight, &clipper, dwColor, points, dwFlags);

	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::DrawText(LONG x, LONG y, const CHAR *pcszStr, DWORD dwStrLen, CMglFont *pFont, COLORREF dwColor, DWORD dwFlags, MGLDRAWTEXTFX* pDrawTextFx)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT clipper;
	MGLBUFFDESC destBuffDesc;
	DWORD dwNativeColor, dwNativeShadowColor;
	FTLIBRARY ftLibrary;
	FTFACE ftFace;
	FTGLYPHCACHE ftGlyphCache;
	DWORD dwFontStyleFlags;
	DWORD dwOpacity, dwShadowOpacity;
	
	// Verify parameters
	if (!pcszStr || !pFont)
		return MGLERR_INVALIDPARAMS;
	
	// Validate font
	if (!pFont->m_font.pFtFace)
		return MGLERR_NOTINITIALIZED;
		
	// Verify the MGL instance
	if (!m_pMgl->IsInitialized())
		return MGLERR_INVALIDINSTANCE;
		
	// Verify opacity FX parameters
	if ((dwFlags & MGLDRAWTEXT_OPACITY) && !pDrawTextFx)
		return MGLERR_INVALIDPARAMS;
		
	// Verify shadow FX parameters
	if ((dwFlags & MGLDRAWTEXT_SHADOW) && !pDrawTextFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if there's no text.
	if (dwStrLen == 0)
		return MGL_OK;
		
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLDRAWTEXT_OPACITY) &&
		pDrawTextFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLDRAWTEXT_OPACITY) &&
		pDrawTextFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLDRAWTEXT_OPACITY;
	}

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Adjust clipper if no clip rectangle is defined
	if (IsRectEmpty(&clipper))
	{
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Init opacity value
	dwOpacity = (dwFlags & MGLDRAWTEXT_OPACITY) ? pDrawTextFx->dwOpacity : 0;
	
	// Init shadow opacity value
	dwShadowOpacity = ((dwFlags & MGLDRAWTEXT_SHADOW) && (dwFlags & MGLDRAWTEXT_OPACITY)) ?
		GET_DT_SHADOW_OPACITY(pDrawTextFx->dwOpacity) : 0;
	
	// -----------------------------------------
	// Set up required font variables
	// -----------------------------------------
	
	// Get the FreeType library pointer from the base class
	ftLibrary = (FTLIBRARY)m_pMgl->m_globals.pFtLibrary;
	
	// Get the fontface
	ftFace = pFont->m_font.pFtFace;
	
	// Get the glyph cache
	ftGlyphCache = (FTGLYPHCACHE)pFont->m_font.pGlyphCache;
	
	// Get the fontface style
	dwFontStyleFlags = pFont->m_font.dwStyle;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	dwNativeShadowColor = (dwFlags & MGLDRAWTEXT_SHADOW) ?
		pxutil565::ColorrefToNative(pDrawTextFx->dwShadowColor): 0;
	
	FT_DrawTextA_565(ftLibrary, ftFace, ftGlyphCache, dwFontStyleFlags,
		pcszStr, dwStrLen, destBuffDesc.pBuffer, destXPitch, destYPitch, x, y,
		destSurfaceWidth, destSurfaceHeight, &clipper, dwNativeColor,
		dwFlags, dwOpacity, dwNativeShadowColor, dwShadowOpacity);
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::DrawText(LONG x, LONG y, const WCHAR *pcwszStr, DWORD dwStrLen, CMglFont *pFont, COLORREF dwColor, DWORD dwFlags, MGLDRAWTEXTFX* pDrawTextFx)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT clipper;
	MGLBUFFDESC destBuffDesc;
	DWORD dwNativeColor, dwNativeShadowColor;
	FTLIBRARY ftLibrary;
	FTFACE ftFace;
	FTGLYPHCACHE ftGlyphCache;
	DWORD dwFontStyleFlags;
	DWORD dwOpacity, dwShadowOpacity;
	
	// Verify parameters
	if (!pcwszStr || !pFont)
		return MGLERR_INVALIDPARAMS;
	
	// Validate font
	if (!pFont->m_font.pFtFace)
		return MGLERR_NOTINITIALIZED;
		
	// Verify the MGL instance
	if (!m_pMgl->IsInitialized())
		return MGLERR_INVALIDINSTANCE;
		
	// Verify opacity FX parameters
	if ((dwFlags & MGLDRAWTEXT_OPACITY) && !pDrawTextFx)
		return MGLERR_INVALIDPARAMS;
		
	// Verify shadow FX parameters
	if ((dwFlags & MGLDRAWTEXT_SHADOW) && !pDrawTextFx)
		return MGLERR_INVALIDPARAMS;
		
	// Optimization: Don't waste time drawing if there's no text.
	if (dwStrLen == 0)
		return MGL_OK;
		
	// Optimization: Don't waste time drawing if opacity is zero.
	if ((dwFlags & MGLDRAWTEXT_OPACITY) &&
		pDrawTextFx->dwOpacity == OPACITY_0)
		return MGL_OK;
		
	// Optimization: Clear OPACITY flag, if not needed.
	if ((dwFlags & MGLDRAWTEXT_OPACITY) &&
		pDrawTextFx->dwOpacity == OPACITY_100)
	{
		dwFlags &= ~MGLDRAWTEXT_OPACITY;
	}

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Adjust clipper if no clip rectangle is defined
	if (IsRectEmpty(&clipper))
	{
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}
	
	// ---------------------------------------------
	
	// Get the destination surface buffer
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// Init opacity value
	dwOpacity = (dwFlags & MGLDRAWTEXT_OPACITY) ? pDrawTextFx->dwOpacity : 0;
	
	// Init shadow opacity value
	dwShadowOpacity = ((dwFlags & MGLDRAWTEXT_SHADOW) && (dwFlags & MGLDRAWTEXT_OPACITY)) ?
		GET_DT_SHADOW_OPACITY(pDrawTextFx->dwOpacity) : 0;
	
	// -----------------------------------------
	// Set up required font variables
	// -----------------------------------------
	
	// Get the FreeType library pointer from the base class
	ftLibrary = (FTLIBRARY)m_pMgl->m_globals.pFtLibrary;
	
	// Get the fontface
	ftFace = pFont->m_font.pFtFace;
	
	// Get the glyph cache
	ftGlyphCache = (FTGLYPHCACHE)pFont->m_font.pGlyphCache;
	
	// Get the fontface style
	dwFontStyleFlags = pFont->m_font.dwStyle;
	
	// -----------------------------------------
	// Perform the blit
	// -----------------------------------------
	LONG destXPitch = destBuffDesc.xPitch / sizeof(WORD);
	LONG destYPitch = destBuffDesc.yPitch / sizeof(WORD);
	
	dwNativeColor = pxutil565::ColorrefToNative(dwColor);
	dwNativeShadowColor = (dwFlags & MGLDRAWTEXT_SHADOW) ?
		pxutil565::ColorrefToNative(pDrawTextFx->dwShadowColor): 0;
	
	FT_DrawTextW_565(ftLibrary, ftFace, ftGlyphCache, dwFontStyleFlags,
		pcwszStr, dwStrLen, destBuffDesc.pBuffer, destXPitch, destYPitch, x, y,
		destSurfaceWidth, destSurfaceHeight, &clipper, dwNativeColor,
		dwFlags, dwOpacity, dwNativeShadowColor, dwShadowOpacity);
	
	// -----------------------------------------
	
	// Release the surface buffer
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::DrawTextEx(RECT *pRect, const CHAR *pcszStr, DWORD dwStrLen, CMglFont *pFont,
	COLORREF dwColor, DWORD dwFlags, MGLDRAWTEXT_EX_FX* pDrawTextExFx, DWORD *pdwTextHeight /*OUT*/)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT rect, clipper;
	MGLBUFFDESC destBuffDesc;
	COLORREF dwBkColor, dwShadowColor;
	DWORD dwOpacity, dwBkOpacity, dwShadowOpacity;
	FTLIBRARY ftLibrary;
	FTFACE ftFace;
	FTGLYPHCACHE ftGlyphCache;
	DWORD dwFontStyleFlags;
	DWORD dwTextHeight;
	
	// --------------------------------------------------
	// Check parameters
	// --------------------------------------------------
	
	// Verify parameters
	if (!pcszStr || !pFont)
		return MGLERR_INVALIDPARAMS;
	
	// Verify BKFILL flag
	if ((dwFlags & MGLDRAWTEXT_EX_BKFILL) && !pDrawTextExFx)
		return MGLERR_INVALIDPARAMS;
		
	// Verify OPACITY flag
	if ((dwFlags & MGLDRAWTEXT_EX_OPACITY) && !pDrawTextExFx)
		return MGLERR_INVALIDPARAMS;
		
	// Verify BKOPACITY flag
	if ((dwFlags & MGLDRAWTEXT_EX_BKOPACITY) && !pDrawTextExFx)
		return MGLERR_INVALIDPARAMS;
		
	// Verify SHADOW flag
	if ((dwFlags & MGLDRAWTEXT_EX_SHADOW) && !pDrawTextExFx)
		return MGLERR_INVALIDPARAMS;
	
	// Validate font
	if (!pFont->m_font.pFtFace)
		return MGLERR_NOTINITIALIZED;
		
	// Verify the MGL instance
	if (!m_pMgl->IsInitialized())
		return MGLERR_INVALIDINSTANCE;
		
	// --------------------------------------------------
	// Perform optimizations
	// --------------------------------------------------
	
	// Clear OPACITY flag, if not needed
	if ((dwFlags & MGLDRAWTEXT_EX_OPACITY) &&
		pDrawTextExFx->dwOpacity == OPACITY_0)
	{
		dwFlags &= ~MGLDRAWTEXT_EX_OPACITY;
	}
	
	// Clear BKOPACITY, if not needed
	if ((dwFlags & MGLDRAWTEXT_EX_BKOPACITY) &&
		pDrawTextExFx->dwBkOpacity == OPACITY_0)
	{
		dwFlags &= ~MGLDRAWTEXT_EX_BKOPACITY;
	}
	
	// --------------------------------------------------
	// Prepare for drawing
	// --------------------------------------------------

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Get destination rectangle
	if (pRect) {
		rect.left = pRect->left;
		rect.top = pRect->top;
		rect.right = pRect->right;
		rect.bottom = pRect->bottom;
	} else {
		rect.left = 0;
		rect.top = 0;
		rect.right = destSurfaceWidth;
		rect.bottom = destSurfaceHeight;
	}
	
	// Verify rectangle
	if (IsRectEmpty(&rect))
		return MGLERR_INVALIDRECT;
	
	// Adjust clipper if no clip rectangle is defined
	if (IsRectEmpty(&clipper))
	{
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}
	else
	{
		// Restrict drawing to the destination rectangle if needed
		if (!(dwFlags & MGLDRAWTEXT_EX_NOCLIP))
		{
			clipper.left = max(clipper.left, rect.left);
			clipper.top = max(clipper.top, rect.top);
			clipper.right = min(clipper.right, rect.right);
			clipper.bottom = min(clipper.bottom, rect.bottom);
		}
	}

	// -----------------------------------------
	// Get the surface buffer
	// -----------------------------------------
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Init FX variables
	// -----------------------------------------
	
	// Init opacity value
	dwOpacity = (dwFlags & MGLDRAWTEXT_EX_OPACITY) ? pDrawTextExFx->dwOpacity : 0;
	
	// Init background opacity value
	dwBkOpacity = (dwFlags & MGLDRAWTEXT_EX_BKOPACITY) ? pDrawTextExFx->dwBkOpacity : 0;
	
	// Init shadow opacity value
	dwShadowOpacity = ((dwFlags & MGLDRAWTEXT_EX_SHADOW) && (dwFlags & MGLDRAWTEXT_EX_OPACITY)) ?
		GET_DT_SHADOW_OPACITY(pDrawTextExFx->dwOpacity) : 0;
	
	// -----------------------------------------
	// Set up required font variables
	// -----------------------------------------
	
	// Get the FreeType library pointer from the base class
	ftLibrary = (FTLIBRARY)m_pMgl->m_globals.pFtLibrary;
	
	// Get the fontface
	ftFace = pFont->m_font.pFtFace;
	
	// Get the glyph cache
	ftGlyphCache = (FTGLYPHCACHE)pFont->m_font.pGlyphCache;
	
	// Get the fontface style
	dwFontStyleFlags = pFont->m_font.dwStyle;
	
	// -----------------------------------------
	// Draw the text
	// -----------------------------------------
	
	dwBkColor = (dwFlags & MGLDRAWTEXT_EX_BKFILL) ? pDrawTextExFx->dwBkColor : 0;
	dwShadowColor = (dwFlags & MGLDRAWTEXT_EX_SHADOW) ? pDrawTextExFx->dwShadowColor : 0;
	
	dwTextHeight = FT_DrawTextExA(ftLibrary, ftFace, ftGlyphCache, dwFontStyleFlags,
		&rect, pcszStr, dwStrLen, &destBuffDesc, &clipper, dwColor, dwFlags, dwBkColor,
		dwOpacity, dwBkOpacity, dwShadowColor, dwShadowOpacity);
		
	// Return text height
	if (pdwTextHeight) *pdwTextHeight = dwTextHeight;

	// -----------------------------------------
	// Release the surface buffer
	// -----------------------------------------
	ReleaseBuffer();

	return MGL_OK;
}

HRESULT CMglSurface::DrawTextEx(RECT *pRect, const WCHAR *pcwszStr, DWORD dwStrLen, CMglFont *pFont,
	COLORREF dwColor, DWORD dwFlags, MGLDRAWTEXT_EX_FX* pDrawTextExFx, DWORD *pdwTextHeight /*OUT*/)
{
	HRESULT hr;
	LONG destSurfaceWidth, destSurfaceHeight;
	RECT rect, clipper;
	MGLBUFFDESC destBuffDesc;
	COLORREF dwBkColor, dwShadowColor;
	DWORD dwOpacity, dwBkOpacity, dwShadowOpacity;
	FTLIBRARY ftLibrary;
	FTFACE ftFace;
	FTGLYPHCACHE ftGlyphCache;
	DWORD dwFontStyleFlags;
	DWORD dwTextHeight;
	
	// --------------------------------------------------
	// Check parameters
	// --------------------------------------------------
	
	// Verify parameters
	if (!pcwszStr || !pFont)
		return MGLERR_INVALIDPARAMS;
	
	// Verify BKFILL flag
	if ((dwFlags & MGLDRAWTEXT_EX_BKFILL) && !pDrawTextExFx)
		return MGLERR_INVALIDPARAMS;
		
	// Verify OPACITY flag
	if ((dwFlags & MGLDRAWTEXT_EX_OPACITY) && !pDrawTextExFx)
		return MGLERR_INVALIDPARAMS;
		
	// Verify BKOPACITY flag
	if ((dwFlags & MGLDRAWTEXT_EX_BKOPACITY) && !pDrawTextExFx)
		return MGLERR_INVALIDPARAMS;
		
	// Verify SHADOW flag
	if ((dwFlags & MGLDRAWTEXT_EX_SHADOW) && !pDrawTextExFx)
		return MGLERR_INVALIDPARAMS;
	
	// Validate font
	if (!pFont->m_font.pFtFace)
		return MGLERR_NOTINITIALIZED;
		
	// Verify the MGL instance
	if (!m_pMgl->IsInitialized())
		return MGLERR_INVALIDINSTANCE;
		
	// --------------------------------------------------
	// Perform optimizations
	// --------------------------------------------------
	
	// Clear OPACITY flag, if not needed
	if ((dwFlags & MGLDRAWTEXT_EX_OPACITY) &&
		pDrawTextExFx->dwOpacity == OPACITY_0)
	{
		dwFlags &= ~MGLDRAWTEXT_EX_OPACITY;
	}
	
	// Clear BKOPACITY, if not needed
	if ((dwFlags & MGLDRAWTEXT_EX_BKOPACITY) &&
		pDrawTextExFx->dwBkOpacity == OPACITY_0)
	{
		dwFlags &= ~MGLDRAWTEXT_EX_BKOPACITY;
	}
	
	// --------------------------------------------------
	// Prepare for drawing
	// --------------------------------------------------

	// Get destination clipper
	hr = GetClipper(&clipper);
	if (FAILED(hr)) return hr;
	
	destSurfaceWidth = GetWidth();
	destSurfaceHeight = GetHeight();
	
	// Get destination rectangle
	if (pRect) {
		rect.left = pRect->left;
		rect.top = pRect->top;
		rect.right = pRect->right;
		rect.bottom = pRect->bottom;
	} else {
		rect.left = 0;
		rect.top = 0;
		rect.right = destSurfaceWidth;
		rect.bottom = destSurfaceHeight;
	}
	
	// Verify rectangle
	if (IsRectEmpty(&rect))
		return MGLERR_INVALIDRECT;
	
	// Adjust clipper if no clip rectangle is defined
	if (IsRectEmpty(&clipper))
	{
		clipper.left = 0;
		clipper.top = 0;
		clipper.right = destSurfaceWidth;
		clipper.bottom = destSurfaceHeight;
	}
	else
	{
		// Restrict drawing to the destination rectangle if needed
		if (!(dwFlags & MGLDRAWTEXT_EX_NOCLIP))
		{
			clipper.left = max(clipper.left, rect.left);
			clipper.top = max(clipper.top, rect.top);
			clipper.right = min(clipper.right, rect.right);
			clipper.bottom = min(clipper.bottom, rect.bottom);
		}
	}

	// -----------------------------------------
	// Get the surface buffer
	// -----------------------------------------
	hr = GetBuffer(&destBuffDesc);
	if (FAILED(hr)) return hr;
	
	// -----------------------------------------
	// Init FX variables
	// -----------------------------------------
	
	// Init opacity value
	dwOpacity = (dwFlags & MGLDRAWTEXT_EX_OPACITY) ? pDrawTextExFx->dwOpacity : 0;
	
	// Init background opacity value
	dwBkOpacity = (dwFlags & MGLDRAWTEXT_EX_BKOPACITY) ? pDrawTextExFx->dwBkOpacity : 0;
	
	// Init shadow opacity value
	dwShadowOpacity = ((dwFlags & MGLDRAWTEXT_EX_SHADOW) && (dwFlags & MGLDRAWTEXT_EX_OPACITY)) ?
		GET_DT_SHADOW_OPACITY(pDrawTextExFx->dwOpacity) : 0;
	
	// -----------------------------------------
	// Set up required font variables
	// -----------------------------------------
	
	// Get the FreeType library pointer from the base class
	ftLibrary = (FTLIBRARY)m_pMgl->m_globals.pFtLibrary;
	
	// Get the fontface
	ftFace = pFont->m_font.pFtFace;
	
	// Get the glyph cache
	ftGlyphCache = (FTGLYPHCACHE)pFont->m_font.pGlyphCache;
	
	// Get the fontface style
	dwFontStyleFlags = pFont->m_font.dwStyle;
	
	// -----------------------------------------
	// Draw the text
	// -----------------------------------------
	
	dwBkColor = (dwFlags & MGLDRAWTEXT_EX_BKFILL) ? pDrawTextExFx->dwBkColor : 0;
	dwShadowColor = (dwFlags & MGLDRAWTEXT_EX_SHADOW) ? pDrawTextExFx->dwShadowColor : 0;
	
	dwTextHeight = FT_DrawTextExW(ftLibrary, ftFace, ftGlyphCache, dwFontStyleFlags,
		&rect, pcwszStr, dwStrLen, &destBuffDesc, &clipper, dwColor, dwFlags, dwBkColor,
		dwOpacity, dwBkOpacity, dwShadowColor, dwShadowOpacity);
		
	// Return text height
	if (pdwTextHeight) *pdwTextHeight = dwTextHeight;

	// -----------------------------------------
	// Release the surface buffer
	// -----------------------------------------
	ReleaseBuffer();

	return MGL_OK;
}