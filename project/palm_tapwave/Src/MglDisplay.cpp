/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"

/**************************************************************
* Globals
**************************************************************/

TwGfxHandle g_hTwGfx = NULL;

/**************************************************************
* Tapwave Helpers
**************************************************************/

#ifndef _WIN32
static asm char _TwGfxOpenNoSignV2 (void *p, void *q)
{
	stmfd sp!, {r4-r11,lr}
	ldr r9, [r9]
	ldr r9, [r9]
	sub sp, sp, #0x24
	mov r6, r0
	mov r7, r1
	ldr pc, =0x200995F0
}
#endif

/**************************************************************
* Internal Functions
**************************************************************/

// CMglDisplay constructor
CMglDisplay::CMglDisplay(CMgl* pMgl) : CMglSurface(pMgl)
{
	TwGfxInfoType info;
	Err err;

	// Open TwGfx library
	info.size = sizeof(TwGfxInfoType);

#ifdef _WIN32
	err = TwGfxOpen(&g_hTwGfx, &info);
#else
	err = _TwGfxOpenNoSignV2 (&g_hTwGfx, &info);
#endif
	ErrFatalDisplayIf(err != errNone, "Error opening TwGfx handle");

	m_pBackBuffer = this;
}

// CMglDisplay destructor
CMglDisplay::~CMglDisplay()
{
	// Close the display
	CloseDisplay();
	
	// Close TwGfx library
	TwGfxClose(g_hTwGfx);
}

// Update the size of the backbuffer to the current size of the display
HRESULT CMglDisplay::ResizeDisplay()
{
	// Has no effect on Tapwave handhelds

	return MGL_OK;
}

// Sets this display object as the primary surface and opens the display for access.
HRESULT CMglDisplay::OpenDisplay(DWORD dwDisplayFlags)
{
	HRESULT hr;
	Err err;
	TwGfxInfoType twInfo;
	DWORD dwSurfaceFlags;
	
	// Get TwGfx info
	twInfo.size = sizeof(TwGfxInfoType);
	err = TwGfxGetInfo(g_hTwGfx, &twInfo);
	if (err != errNone) MGLERR_INVALIDPARAMS;
	
	// Initialize backbuffer surface flags
	dwSurfaceFlags = MGLSURFACE_CLEAR;
	if (dwDisplayFlags & MGLDISPLAY_BACKBUFFERVIDMEM)
		dwSurfaceFlags |= MGLSURFACE_VIDEOMEMORY;
	else
		dwSurfaceFlags |= MGLSURFACE_SYSTEMMEMORY;

	// Create backbuffer surface. The backbuffer MUST be
	// in video memory; otherwise, Flip() will not work correctly.
	hr = CreateSurface(dwSurfaceFlags, twInfo.displayWidth, twInfo.displayHeight);
	if (FAILED(hr)) return hr;
	
	// Set backbuffer as primary surface
	m_surface.dwFlags |= MGLSURFACE_PRIMARY;

	return MGL_OK;
}

// Closes the display. The display is automatically closed in the CMglDisplay destructor.
HRESULT CMglDisplay::CloseDisplay()
{
	return MGL_OK;
}

// Returns a pointer to the back buffer of the display
// Note: Do not delete this pointer! It will be automatically freed by CMglDisplay.
CMglSurface* CMglDisplay::GetBackBuffer()
{
	return m_pBackBuffer;
}

// Returns the total and free amount of video memory
HRESULT CMglDisplay::GetAvailableVidMem(DWORD dwFlags, DWORD* pTotal, DWORD* pFree)
{
	Err err;
	TwGfxInfoType twInfo;
	Int32 usedMem;
	
	// Get total VRAM memory
	twInfo.size = sizeof(TwGfxInfoType);
	err = TwGfxGetInfo(g_hTwGfx, &twInfo);
	if (err != errNone) MGLERR_INVALIDPARAMS;
	
	// Get VRAM memory usage
	err = TwGfxGetMemoryUsage(g_hTwGfx, twGfxLocationAcceleratorMemory, &usedMem);
	if (err != errNone) MGLERR_INVALIDPARAMS;
	
	// Return requested values
	if (pTotal) *pTotal = (DWORD)twInfo.totalAcceleratorMemory;
	if (pFree)	*pFree = (DWORD)twInfo.totalAcceleratorMemory - (DWORD)usedMem;

	return MGL_OK;
}

// Returns the hw info of the video device
HRESULT CMglDisplay::GetHWInfo(DWORD* pInfo)
{
	// Verify parameters
	if (!pInfo) return MGLERR_INVALIDPARAMS;
	
	// Return HW info. Tapwave Zodiac supports VIDMEM
	// along with VSYNC.
	*pInfo = MGLVIDEOHW_VIDMEM | MGLVIDEOHW_VSYNC;

	return MGL_OK;
}

// Copy the back buffer to screen. Access the backbuffer using GetBackBuffer().
HRESULT CMglDisplay::Flip()
{
	TwGfxSurfaceHandle twDisplayHandle;
	const TwGfxPointType twDestPt = {0, 0};
	TwGfxRectType twSrcRect;
	TwGfxBitmapType twBmp;
	Err err;
	
	// Verify the backbuffer surface
	if (m_surface.dwSize == 0)
		return MGLERR_NOTINITIALIZED;
		
	// Ensure backbuffer surface isn't locked
	if (m_surface.dwFlags & (MGLSURFACE_LOCKED |	
							MGLSURFACE_VIDEOLOCKED))
		return MGLERR_LOCKEDSURFACES;
	
	// Wait for a vertical blank
	//TwGfxWaitForVBlank(g_hTwGfx);
	
	// Flip surface stored in video memory
	if (m_surface.dwFlags & MGLSURFACE_VIDEOMEMORY)
	{
		// Get the TwGfx HW display surface handle
		err = TwGfxGetDisplaySurface(g_hTwGfx, &twDisplayHandle);
		if (err != errNone) return MGLERR_INVALIDPARAMS;
		
		twSrcRect.x = 0;
		twSrcRect.y = 0;
		twSrcRect.w = (Int32)m_surface.dwWidth;
		twSrcRect.h = (Int32)m_surface.dwHeight;
		
		// Blit the MojaveGL backbuffer surface to the TwGfx display surface
		err = TwGfxBitBlt(twDisplayHandle, &twDestPt,
			(TwGfxSurfaceHandle)m_surface.pBuffer, &twSrcRect);
		if (err != errNone)
		{
			if (err == twGfxErrorOperationInProgress)
				return MGLERR_SURFACEBUSY;
			else
				return MGLERR_INVALIDPARAMS;
		}
	}
	// Flip surface stored in dynamic memory
	else // if (m_surface.dwFlags & MGLSURFACE_SYSTEMEMORY)
	{
		// Get the TwGfx HW display surface handle
		err = TwGfxGetPalmDisplaySurface(g_hTwGfx, &twDisplayHandle);
		if (err != errNone) return MGLERR_INVALIDPARAMS;
		
		twBmp.size = sizeof(TwGfxBitmapType);
		twBmp.width = (Int32)m_surface.dwWidth;
		twBmp.height = (Int32)m_surface.dwHeight;
		twBmp.rowBytes = m_surface.yPitch;
		twBmp.pixelFormat = twGfxPixelFormatRGB565_LE;
		twBmp.data = m_surface.pBuffer;
		twBmp.palette = NULL;
		
		TwGfxDrawBitmap(twDisplayHandle, &twDestPt, &twBmp);
	}
	
	return MGL_OK;
}