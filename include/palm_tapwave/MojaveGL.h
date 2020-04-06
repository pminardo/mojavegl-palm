/**************************************************************
* MojaveGL Graphics Library v1.0 (BETA)
* Written by Parker Minardo 
*
* Copyright (c) 2006-2007 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __MOJAVEGL_H_
#define __MOJAVEGL_H_

/**************************************************************
* Platform Includes
**************************************************************/
#include "MojaveGLDefines.h"

/**************************************************************
* Prototypes
**************************************************************/

class CMgl;
class CMglSurface;
class CMglFont;
class CMglDisplay;

// Saguaro redefines these MojaveGL constants and
// structures in mgl.h. Don't define them here
// if we're building against Saguaro. This keeps
// the Saguaro gfx code much cleaner... - ptm 1/11/07
#ifndef _BUILD_SAGUARO

/**************************************************************
* Function Parameters
**************************************************************/

enum {

	// -------------------- OpenDisplay() flags ---------------------
	
	MGLDISPLAY_BACKBUFFERVIDMEM	= 0x0001, // Use a back buffer stored in video memory. (Tapwave Zodiac)

	// -------------------- Video HW info ---------------------
	
	MGLVIDEOHW_VIDMEM               = 0x0001, // Video memory is available and enabled
	MGLVIDEOHW_VSYNC                = 0x0002, // Video HW can sync flip to vertical blank
	
	// -------------------- Pixel formats ---------------------
	
	MGLPIXELFORMAT_444	= 0x0001, // 16-bit RGB  (xxxxrrrr ggggbbbb)
	MGLPIXELFORMAT_555              = 0x0002, // 16-bit RGB  (xrrrrrgg gggbbbbb)
	MGLPIXELFORMAT_565              = 0x0004, // 16-bit RGB  (rrrrrggg gggbbbbb) 
	MGLPIXELFORMAT_444A             = 0x0010, // 16-bit RGBA (rrrrgggg bbbbaaaa)
	MGLPIXELFORMAT_555A             = 0x0020, // 32-bit RGBA (xrrrrrgg gggbbbbb xxxxxxxx aaaaaaaa)
	MGLPIXELFORMAT_565A             = 0x0040, // 32-bit RGBA (rrrrrggg gggbbbbb xxxxxxxx aaaaaaaa)
	
	// -------------------- CreateSurface() flags ---------------------
	
	MGLSURFACE_CLEAR                = 0x0001, // The surface is cleared after creation
	MGLSURFACE_READONLY             = 0x0002, // This is a read-only surface. On Palm OS, pixel data in stored in the storage heap.
	MGLSURFACE_SYSTEMMEMORY         = 0x0100, // The surface is stored in system memory
	MGLSURFACE_VIDEOMEMORY          = 0x0200, // The surface is stored in video memory
	MGLSURFACE_LOCALVIDMEM          = 0x0400, // The surface is stored in local video memory
	MGLSURFACE_NONLOCALVIDMEM       = 0x0800, // The surface is stored in non local video memory

	// -------------------- Surface flags set internally by MojaveGL ---------------------

	MGLSURFACE_LOCKED               = 0x1000, // The surface is locked
	MGLSURFACE_VIDEOLOCKED          = 0x2000, // The surface is video locked
	MGLSURFACE_PRIMARY              = 0x4000, // The display surface can be drawn to directly
	
	// -------------------- CreateFont() flags ---------------------
	
	MGLFONT_LOCALCOPY		= 0x0001, // Make a local copy of the font data
	MGLFONT_READONLY			= 0x0002, // The font data is read-only. Combine with LOCALCOPY flag to store font data in the storage heap (on Palm OS).
	MGLFONT_NOCACHE			= 0x0004,	// Disable caching of font glyphs to reduce (dynamic heap) memory usage
	MGLFONT_TINYCACHE			= 0x0008,	// Cache at most 32 font glyphs (as opposed to unlimited).
	
	// -------------------- Font style flags ---------------------
	
	MGLFONTSTYLE_BOLD			= 0x0001, // The font is BOLD
	MGLFONTSTYLE_ITALIC		= 0x0002,	// The font is ITALIC
	MGLFONTSTYLE_ANTIALIAS	= 0x1000, // The font is antialiased
	MGLFONTSTYLE_NOHINTING	= 0x2000, // Disable font hinting
	
	// -------------------- BitBlt() flags ---------------------
	
	MGLBITBLT_KEYSRC			= 0x0001,	// Masks out the colors specified by KEYSRC before blitting
	MGLBITBLT_COLORFILL            = 0x0002, // Uses the specified color instead of source image color
	
	// -------------------- StretchBlt() flags ---------------------
	
	MGLSTRETCHBLT_MIRRORLEFTRIGHT 	= 0x0001, // Mirrors the source image left-right
	MGLSTRETCHBLT_MIRRORUPDOWN		= 0x0002, // Mirrors the source image upside down
	MGLSTRETCHBLT_KEYSRC				= 0x0004,	// Masks out the colors specified by KEYSRC before blitting
	MGLSTRETCHBLT_COLORFILL            		= 0x0008, // Uses the specified color instead of source image color
	
	// -------------------- RotateBlt() flags ---------------------
	
	MGLROTATEBLT_CENTER				= 0x0001,	// destX and destY become center point for rotation
	MGLROTATEBLT_KEYSRC				= 0x0002,	// Masks out the colors specified by KEYSRC before blitting
	MGLROTATEBLT_COLORFILL            		= 0x0004, // Uses the specified color instead of source image color
	
	// -------------------- AlphaBlt() flags ---------------------
	
	MGLALPHABLT_OPACITY		= 0x0001, // Specifies opacity to adjust the overall weight of the alpha blend (0 transparent - 255 opaque)
	
	// -------------------- StretchAlphaBlt() flags ---------------------
	
	MGLSTRETCHALPHABLT_MIRRORLEFTRIGHT 	= 0x0001, // Mirrors the source image left-right
	MGLSTRETCHALPHABLT_MIRRORUPDOWN	= 0x0002, // Mirrors the source image upside down
	MGLSTRETCHALPHABLT_OPACITY			= 0x0004, // Specifies opacity to adjust the overall weight of the alpha blend (0 transparent - 255 opaque)
	
	// -------------------- SetPixels() flags ---------------------
	
	MGLSETPIXELS_NOCOORDCHECK         = 0x0001, // Disable coordinate verifications to optimize speed
	MGLSETPIXELS_FIXEDPOINT         = 0x0002, // Use 16:16 fixed point coordinates
	
	// -------------------- MGLPIXEL flags ---------------------
	
	MGLPIXEL_OPACITY                = 0x0001, 	// Uses opacity to blend the pixel with the background 
	MGLPIXEL_DISABLED               = 0x0002, // Do not draw this pixel
	
	// -------------------- DrawLine() flags ---------------------
	
	MGLDRAWLINE_OPACITY             = 0x0001, // Specifies opacity of the line (0 transparent - 255 opaque)
	MGLDRAWLINE_ANTIALIAS             = 0x0002, // Use a fast fixed-point Wu line algorithm to draw an antialiased line
	
	// -------------------- DrawRect() flags ---------------------
	
	MGLDRAWRECT_OPACITY             = 0x0001, // Specifies opacity of the rectangle (0 transparent - 255 opaque)
	
	// -------------------- FillRect() flags ---------------------
	
	MGLFILLRECT_OPACITY             = 0x0001, // Specifies opacity of the rectangle (0 transparent - 255 opaque)
	
	// -------------------- FillGradient() flags ---------------------
	
	MGLFILLGRADIENT_HORIZ		= 0x0001,	// Draw a horizontal gradient
	MGLFILLGRADIENT_VERT		= 0x0002,	// Draw a vertical gradient
	MGLFILLGRADIENT_OPACITY	= 0x0004,	// Specifies opacity of the gradient (0 transparent - 255 opaque)
	MGLFILLGRADIENT_DITHER	= 0x0008,	// Use dithering when drawing the gradient
	
	// -------------------- DrawEllipse() flags ---------------------
	
	MGLDRAWELLIPSE_OPACITY	= 0x0001, // Specifies opacity of the ellipse (0 transparent - 255 opaque)
	
	// -------------------- FillEllipse() flags ---------------------
	
	MGLFILLELLIPSE_OPACITY		= 0x0001, // Specifies opacity of the ellipse (0 transparent - 255 opaque)
	
	// -------------------- DrawPolyline() flags ---------------------
	
	MGLDRAWPOLYLINE_ANTIALIAS   = 0x0001, // Use a fast fixed-point Wu line algorithm for antialiasing
	
	// -------------------- DrawPolygon() flags ---------------------
	
	MGLDRAWPOLYGON_ANTIALIAS   = 0x0001, // Use a fast fixed-point Wu line algorithm for antialiasing
	
	// -------------------- FillPolygon() flags ---------------------
	
	MGLFILLPOLYGON_OPACITY   = 0x0001, // Specifies opacity of the polygon (0 transparent - 255 opaque)
	
	// -------------------- DrawBezier() flags ---------------------
	
	MGLDRAWBEZIER_ANTIALIAS             = 0x0001, // Use a fast fixed-point Wu line algorithm for antialiasing
	
	// -------------------- DrawText() flags ---------------------
	
	MGLDRAWTEXT_OPACITY   = 0x0001, // Specifies opacity of the text (0 transparent - 255 opaque)
	MGLDRAWTEXT_SHADOW	= 0x0002,	// Draw the text with a quick shadow; when combined with MGLDRAWTEXT_OPACITY the shadow's opacity is computed by the formula = (opacity-(opacity/3))
	
	// -------------------- DrawTextEx() flags ---------------------
	
	MGLDRAWTEXT_EX_NOCLIP	= 0x0001,	// Don't clip to draw rectangle; clip to entire destination surface instead
	MGLDRAWTEXT_EX_BKFILL	= 0x0002,	// Perform a background fill
	MGLDRAWTEXT_EX_WORDELLIPSIS	= 0x0004, // Truncates any word that does not fit in the rectangle, drawing an ellipsis to indicate the truncation
	MGLDRAWTEXT_EX_WORDWRAP	= 0x0008,	// Perform word-wrapping on the text string
	MGLDRAWTEXT_EX_OPACITY		= 0x0010, // Specifies opacity of the text (0 transparent - 255 opaque)
	MGLDRAWTEXT_EX_BKOPACITY	= 0x0020,	// Specifies opacity of the background fill (0 transparent - 255 opaque)
	MGLDRAWTEXT_EX_SHADOW		= 0x0040,	// Draw the text with a quick shadow; when combined with MGLDRAWTEXT_EX_OPACITY the shadow's opacity is computed by the formula = (opacity-(opacity/3))
	
	// -------------------- Misc. flags ---------------------
	
	MGLSTRLEN_INFINITE		= 0xFFFFFFFFL,	// Use the entire string for the operation
	
	// -------------------- Error codes ---------------------
	
	// OK
	MGL_OK                             = S_OK,
	// OUTOFMEMORY: Memory allocation failed.
	MGLERR_OUTOFMEMORY                 = E_OUTOFMEMORY,
	// INVALIDPARAMS: One or more arguments are either NULL or contain invalid values.
	MGLERR_INVALIDPARAMS               = E_INVALIDARG,
	// NOTLOCKED: The surface is not locked, and cannot be unlocked.
	MGLERR_NOTLOCKED                   = ((HRESULT)0xC8660010),
	// BITMAPWRITEERROR: The bitmap file could not be written.
	MGLERR_BITMAPWRITEERROR            = ((HRESULT)0xC8660020),
	// BITMAPNOTFOUND: The specified bitmap could not be found.
	MGLERR_BITMAPNOTFOUND              = ((HRESULT)0xC8660030),
	// INVALIDBITMAP: The bitmap file could not be parsed.
	MGLERR_INVALIDBITMAP               = ((HRESULT)0xC8660040),
	// NOTINITIALIZED: All objects must be created before use (OpenDisplay/CreateSurface).
	MGLERR_NOTINITIALIZED              = ((HRESULT)0xC8660050),
	// INVALIDSURFACETYPE: Cannot call CreateSurface on objects of type CMglDisplay.
	MGLERR_INVALIDSURFACETYPE          = ((HRESULT)0xC8660060),
	// INCOMPATIBLEPRIMARY: Primary surface does not exist or is aligned different to the current surface.
	MGLERR_INCOMPATIBLEPRIMARY         = ((HRESULT)0xC8660070),
	// PRIMARYSURFACEALREADYEXISTS: Cannot call OpenDisplay if a primary surface already has been assigned.
	MGLERR_PRIMARYSURFACEALREADYEXISTS = ((HRESULT)0xC8660080),
	// LOCKEDSURFACES: One or more surfaces are locked, preventing operation.
	MGLERR_LOCKEDSURFACES              = ((HRESULT)0xC8660090),
	// INVALIDRECT: One or more rectangles are invalid.
	MGLERR_INVALIDRECT                 = ((HRESULT)0xC86600a0),
	// MGLERR_INVALIDMODE: Invalid display mode
	MGLERR_INVALIDMODE                 = ((HRESULT)0xC86600b0),
	// MGLERR_UNSUPPORTEDMODE: Unsupported display resolution or incompatible device
	MGLERR_UNSUPPORTEDMODE             = ((HRESULT)0xC86600c0),
	// MGLERR_NOVIDEOHW: The device does not have any video acceleration
	MGLERR_NOVIDEOHW                   = ((HRESULT)0xC86600d0),
	// MGLERR_NOVIDEOSURFACE: The surface is not stored in video memory
	MGLERR_NOVIDEOSURFACE              = ((HRESULT)0xC86600e0),
	// MGLERR_SURFACELOST: The contents of a video surface was destroyed
	MGLERR_SURFACELOST                 = ((HRESULT)0xC86600f0),
	// MGLERR_SURFACEBUSY: A surface is already busy with a hardware blit operation
	MGLERR_SURFACEBUSY                 = ((HRESULT)0xC8660100),
	//MGLERR_READONLYSURFACE: This is a read-only surface
	MGLERR_READONLYSURFACE			  = ((HRESULT)0xC8660110),
	//MGLERR_ALREADYINITIALIZED: The object is already initialized.
	MGLERR_ALREADYINITIALIZED			= ((HRESULT)0xC8660120),
	//MGLERR_INVALIDINSTANCE: The MojaveGL instance is invalid.
	MGLERR_INVALIDINSTANCE			= ((HRESULT)0xC8660130),
	//MGLERR_INVALIDFONT: The font file could not be parsed.
	MGLERR_INVALIDFONT			= ((HRESULT)0xC8660140)
};

/**************************************************************
* Structures
**************************************************************/

// Surface description returned from GetBuffer
typedef struct _MGLBUFFDESC {
	DWORD     dwWidth;       // Buffer width in pixels
	DWORD     dwHeight;      // Buffer height in pixels
	LONG      xPitch;        // xPitch in bytes
	LONG      yPitch;        // yPitch in bytes
	void*     pBuffer;		 // Buffer pointer
	DWORD     dwPixelFormat; // Pixel format - currently MGLPIXELFORMAT_555 and MGLPIXELFORMAT_565 are supported
} MGLBUFFDESC;

// BitBlt FX structure
typedef struct _MGLBITBLTFX {
	COLORREF  dwFillColor;       // Uses the specified color instead of source image color
} MGLBITBLTFX;

// StretchBlt FX structure
typedef struct _MGLSTRETCHBLTFX {
	COLORREF  dwFillColor;       // Uses the specified color instead of source image color
} MGLSTRETCHBLTFX;

// RotateBlt FX structure
typedef struct _MGLROTATEBLTFX {
	COLORREF  dwFillColor;       // Uses the specified color instead of source image color
} MGLROTATEBLTFX;

// AlphaBlt FX structure
typedef struct _MGLALPHABLTFX {
	DWORD     dwOpacity;         // Specifies overall weight opacity to use for the alpha blend (0 transparent - 255 opaque)
} MGLALPHABLTFX;

// StretchAlphaBlt FX structure
typedef struct _MGLSTRETCHALPHABLTFX {
	DWORD     dwOpacity;         // Specifies overall weight opacity to use for the alpha blend (0 transparent - 255 opaque)
} MGLSTRETCHALPHABLTFX;

typedef struct _MGLPIXELFX {
	DWORD     dwOpacity;         // Specifies opacity of each pixel (0 transparent - 128 (50% quick alpha) - 255 opaque)
} MGLPIXELFX;

// Pixel structure used by SetPixels
typedef struct _MGLPIXEL {
	LONG       x;
	LONG       y;
	COLORREF   dwColor;
	DWORD      dwFlags;
	MGLPIXELFX  pixelfx;
} MGLPIXEL;

typedef struct _MGLPIXELNODE {
	MGLPIXEL       pixel;
	_MGLPIXELNODE	*pNext;
} MGLPIXELNODE;

// DrawLine FX structure
typedef struct _MGLDRAWLINEFX {
	DWORD     dwOpacity;         // Specifies opacity of the line (0 transparent - 128 (50% quick alpha) - 255 opaque)
} MGLDRAWLINEFX;

// DrawRect FX structure
typedef struct _MGLDRAWRECTFX {
	DWORD     dwOpacity;         // Specifies opacity of the rectangle (0 transparent - 128 (50% quick alpha) - 255 opaque)
} MGLDRAWRECTFX;

// FillRect FX structure
typedef struct _MGLFILLRECTFX {
	DWORD     dwOpacity;         // Specifies opacity of the filled rectangle (0 transparent - 128 (50% quick alpha) - 255 opaque)
} MGLFILLRECTFX;

// FillGradient FX structure
typedef struct _MGLFILLGRADIENTFX {
	DWORD     dwOpacity;         // Specifies opacity of the gradient (0 transparent - 128 (50% quick alpha) - 255 opaque)
} MGLFILLGRADIENTFX;

// DrawEllipse FX structure
typedef struct _MGLDRAWELLIPSEFX {
	DWORD     dwOpacity;         // Specifies opacity of the ellipse (0 transparent - 128 (50% quick alpha) - 255 opaque)
} MGLDRAWELLIPSEFX;

// FillEllipse FX structure
typedef struct _MGLFILLELLIPSEFX {
	DWORD     dwOpacity;         // Specifies opacity of the ellipse (0 transparent - 128 (50% quick alpha) - 255 opaque)
} MGLFILLELLIPSEFX;

// DrawPolyline FX structure
typedef struct _MGLDRAWPOLYLINEFX {
	DWORD dwReserved;	// reserved for future use
} MGLDRAWPOLYLINEFX;

// DrawPolygon FX structure
typedef struct _MGLDRAWPOLYGONFX {
	DWORD dwReserved;	// reserved for future use
} MGLDRAWPOLYGONFX;

// FillPolygon FX structure
typedef struct _MGLFILLPOLYGONFX {
	DWORD     dwOpacity;         // Specifies opacity of the polygon (0 transparent - 128 (50% quick alpha) - 255 opaque)
} MGLFILLPOLYGONFX;

// DrawBezier FX structure
typedef struct _MGLDRAWBEZIER {
	DWORD dwReserved;	// reserved for future use
} MGLDRAWBEZIER;

// DrawText FX structure
typedef struct _MGLDRAWTEXTFX {
	DWORD	dwOpacity;	 // Specifies opacity of the text (0 transparent - 128 (50% quick alpha) - 255 opaque)
	COLORREF dwShadowColor;	// Specifies color of the shadow
} MGLDRAWTEXTFX;

// DrawTextEx FX structure
typedef struct _MGLDRAWTEXT_EX_FX {
	COLORREF dwBkColor;		// Specifies background fill color
	DWORD dwOpacity;		// Specifies opacity of the text (0 transparent - 128 (50% quick alpha) - 255 opaque)
	DWORD dwBkOpacity;		// Specifies opacity of the background (0 transparent - 128 (50% quick alpha) - 255 opaque)
	COLORREF dwShadowColor;	// Specifies color of the shadow
} MGLDRAWTEXT_EX_FX;

// Text metrics structure
typedef struct _MGLTEXTMETRICS {
	LONG nAscent;
	LONG nDescent;
	LONG nHeight;
	LONG nMaxHorizAdvance;
} MGLTEXTMETRICS;

// Text bounding box structure
typedef struct _MGLBBOX {
	DWORD dwWidth;
	DWORD dwHeight;
} MGLBBOX;

typedef struct _MGLTEXTLINENODE {
	DWORD dwStartOffset;
	DWORD dwLength;
	_MGLTEXTLINENODE *pNext;
} MGLTEXTLINENODE;

#endif // _BUILD_SAGUARO

/**************************************************************
* Internal Structures
**************************************************************/

// Bitmap decoder uses these structs
typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
        WORD	_padding;
} BITMAPFILEHEADER;

#define SIZEOF_BITMAPFILEHEADER	14

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

/* Constants for the biCompression field */
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L

typedef struct tagBITMAPINFO { 
  BITMAPINFOHEADER bmiHeader; 
  RGBQUAD          bmiColors[1]; 
} BITMAPINFO; 

namespace mglinternal {

	// MojaveGL globals
	typedef struct _MGLGLOBALS
	{
		void *pFtLibrary;	// FT_Library
	} MGLGLOBALS;

	// Generic surface format
	typedef struct _MGLSURFACE
	{
		DWORD     dwFlags;
		DWORD     dwWidth;
		DWORD     dwHeight;
		LONG      xPitch;
		LONG      yPitch;
		DWORD     dwSize;
		COLORREF  ckSrcBlt;
		RECT clipper;
		void*	  pBuffer;	// malloc'd() data or TwGfxSurfaceType* (depending on surface flags)
		void*   pVideoAddr;	// used for LockVideoSurface()
	
		// Palm specific
		DWORD	  dwCreatorID;
		LONG	  ftrNum;
	} MGLSURFACE;
	
	// Generic font format
	typedef struct _MGLFONT
	{
		DWORD dwFlags;
		DWORD dwStyle;
		DWORD dwCharWidth;
		DWORD dwCharHeight;
		DWORD dwHorizDpi;
		DWORD dwVertDpi;
		LONG nAscent;
		LONG nDescent;
		LONG nHeight;
		LONG nMaxHorizAdvance;
		void *pFtFace;	// FT_Face
		void *pBuffer;	// pointer to font data
		void *pGlyphCache;	// FT_CacheHandle
		
		// Palm specific
		DWORD dwCreatorID;
		LONG ftrNum;
	} MGLFONT;
}

/**************************************************************
* MojaveGL Math
**************************************************************/

namespace mglmath {

	// Return cosine of x.
	DOUBLE cos(DOUBLE x);
	
	// Return sine of x.
	DOUBLE sin(DOUBLE x);
	
	// Return sqrt of x.
	DOUBLE sqrt(DOUBLE x);
}

#ifndef PI
#define PI		3.14159265f
#endif

// Converts degrees to radians.
#define DEG2RAD(d)		(((d)*PI)/180.0f)

// Converts radians to degrees.
#define RAD2DEG(r)    		(((r)*180.0f)/PI)

/**************************************************************
* Classes
**************************************************************/

class CMgl {

	// Construction
	public:
		CMgl();
		
	// Attributes
	protected:
	mglinternal::MGLGLOBALS m_globals;
	friend class CMglSurface;
	friend class CMglFont;
	friend class CMglDisplay;
	
	// Operations
	public:
		// Initialize the MojaveGL instance.
		HRESULT Initialize();
		
		// Returns TRUE if the MojaveGL instance
		// is initialized.
		BOOL IsInitialized();
		
		// Closes the MojaveGL instance. This function
		// is called automatically by the CMgl destructor.
		HRESULT FreeInstance();

	// Implementation
	public:
		virtual ~CMgl();
};

class CMglSurface {

	// Construction
	public:
		CMglSurface(CMgl* pMgl);

	// Attributes
	protected:
		CMgl* m_pMgl;
		friend class CMglDisplay;
		mglinternal::MGLSURFACE m_surface;
	
	public:
	
		// -------------------- Surface Initialization ---------------------
		
		// Creates a new surface as a copy of the specified source surface
		HRESULT CreateSurface(CMglSurface* pSrcSurface);
		
		// Creates a new surface of the specified size.
		HRESULT CreateSurface(DWORD dwFlags, DWORD dwWidth, DWORD dwHeight);
		
		// Create a new surface from a bitmap resource
		HRESULT CreateSurface(DWORD dwFlags, DWORD dwResourceID);
		
		// Creates a new surface from the specified image file in memory.
		// This function currently only supports *.bmp and *.dib file formats.
		HRESULT CreateSurface(DWORD dwFlags, BYTE* pImageFileMem, DWORD dwImageFileSize);

		// Frees all resources associated with the surface object. This function
		// is called automatically by the destructor.
		HRESULT FreeSurface();
		
		// -------------------- Surface Operations ---------------------
		
		HRESULT SetClipper(RECT* pRect);
		HRESULT GetClipper(RECT* pRect);
	
		DWORD GetWidth();
		DWORD GetHeight();

		HRESULT GetColorKey(COLORREF* pColorKey);
		HRESULT SetColorKey(COLORREF dwColorKey);

		HRESULT GetBuffer(MGLBUFFDESC* pBuffDesc);
		HRESULT ReleaseBuffer();

		HRESULT GetSurfaceFlags(DWORD* pFlags);
		
		// -------------------- HW Operations ---------------------
		
		// Locks a surface stored in video memory. If a surface is stored in video
		// memory it has to be locked exclusively on each operation that is not
		// hardware accelerated. Locking a surface involves much overhead,
		// and should be avoided if possible. Call LockVideoSurface() if you intend
		// to use several consecutive non-accelerated operations on surfaces
		// stored in video memory. Be sure to keep surfaces locked for as 
		// little amount of time as possible.
		HRESULT LockVideoSurface();
		HRESULT UnlockVideoSurface();
		
		// -------------------- Blit Operations ---------------------
		
		// Performs a highly optimized 1:1 blit (source to destination).
		// This function is equivalent to BltFast().
		// Depending on the surface alignment, this function will either:
		// 	1) Read and write 32-bit pixel data
		//	2) Read 32-bit pixel data and write 16-bit pixel data
		//	3) Read and write 16-bit pixel data
		// This function is hardware-accelerated on the Tapwave Zodiac
		// when both the source and destination surfaces are stored in
		// video memory. KEYSRC operation is also hardware-accelerated.
		HRESULT BitBlt(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwFlags, MGLBITBLTFX *pBitBltFx);
		HRESULT BitBltOpacity(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwOpacity, DWORD dwFlags, MGLBITBLTFX *pBitBltFx);
		
		// Performs a tiled BitBlt. On Palm OS 5, this function is faster
		// than calling BitBlt repeatedly due to the PceNativeCall overhead.
		HRESULT TileBitBlt(RECT *pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwFlags, MGLBITBLTFX *pBitBltFx);
		HRESULT TileBitBltOpacity(RECT *pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwOpacity, DWORD dwFlags, MGLBITBLTFX *pBitBltFx);
		
		// Performs a stretched blit.
		HRESULT StretchBlt(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwFlags, MGLSTRETCHBLTFX* pStretchBltFx);
		HRESULT StretchBltOpacity(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwOpacity, DWORD dwFlags, MGLSTRETCHBLTFX* pStretchBltFx);
		
		// Performs a rotated blit. The rotation occurs around destX and destY.
		// nAngle is in radians (-3.14 to 3.14). If you prefer to specify the angle
		// as degrees, you can use the DEG2RAD macro defined by MojaveGL.
		HRESULT RotateBlt(LONG destX, LONG destY, FLOAT nAngle, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwFlags, MGLROTATEBLTFX* pRotateBltFx);
		HRESULT RotateBltOpacity(LONG destX, LONG destY, FLOAT nAngle, CMglSurface* pSrcSurface, RECT* pSrcRect, DWORD dwOpacity, DWORD dwFlags, MGLROTATEBLTFX* pRotateBltFx);
		
		// -------------------- Alpha Blending ---------------------
		
		// Performs a highly optimized 1:1 alpha blend blit.
		// This function is equivalent to AlphaBltFast().
		// AlphaBltAdditive() and AlphaBltSubtractive() take
		// the same flags and MGLALPHABLTFX struct as the regular AlphaBlt().
		// Depending on the surface alignment, this function will either:
		// 	1) Read and write 32-bit pixel data
		//	2) Read 32-bit pixel data and write 16-bit pixel data
		//	3) Read and write 16-bit pixel data
		// This function is not hardware-accelerated.
		HRESULT AlphaBlt(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT *pAlphaRect, DWORD dwFlags, MGLALPHABLTFX *pAlphaBltFx);
		HRESULT AlphaBltAdditive(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT *pAlphaRect, DWORD dwFlags, MGLALPHABLTFX *pAlphaBltFx);
		HRESULT AlphaBltSubtractive(LONG destX, LONG destY, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT *pAlphaRect, DWORD dwFlags, MGLALPHABLTFX *pAlphaBltFx);
		
		// Performs a stretched AlphaBlt. This function is not hardware-accelerated.
		// StretchAlphaBltAdditive() and StretchAlphaBltSubtractive() take
		// the same flags and MGLSTRETCHALPHABLTFX struct as the regular StretchAlphaBlt().
		HRESULT StretchAlphaBlt(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT* pAlphaRect, DWORD dwFlags, MGLSTRETCHALPHABLTFX* pStretchAlphaBltFx);
		HRESULT StretchAlphaBltAdditive(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT* pAlphaRect, DWORD dwFlags, MGLSTRETCHALPHABLTFX* pStretchAlphaBltFx);
		HRESULT StretchAlphaBltSubtractive(RECT* pDestRect, CMglSurface* pSrcSurface, RECT* pSrcRect, CMglSurface *pAlphaSurface, RECT* pAlphaRect, DWORD dwFlags, MGLSTRETCHALPHABLTFX* pStretchAlphaBltFx);
		
		// -------------------- Drawing Tools ---------------------
		
		// Returns the color value of a pixel.
		HRESULT GetPixel(LONG x, LONG y, COLORREF* pColor);

		// Sets a single pixel. This function respects the defined clipping rectangle
		// and will not set the pixel if coordinates outside of the clipping rectangle
		// are specified.
		HRESULT SetPixel(LONG x, LONG y, COLORREF dwColor);
		
		// Draw multiple pixels from an array/linked list. If the MGLSETPIXELS_NOCOORDCHECK
		// flag is specified, this function performs no coordinate verification as a
		// speed optimization. When this flag is specified, you must ensure
		// all coordinates specified are valid (x >= 0 && y >= 0 && x < GetWidth() && y < GetHeight())
		// otherwise, the results will be unpredictable.
		HRESULT SetPixels(MGLPIXEL *pPixelArray, DWORD dwElementSize, DWORD dwElementCount, DWORD dwFlags);
		HRESULT SetPixels(MGLPIXELNODE *pHead, DWORD dwFlags);
		
		// Draws a line. This function is highly optimized to use
		// one of the following methods, depending on the
		// coordinates specified:
		//	1) Draw a vertical line when (x1 == x2)
		//	2) Draw a horizontal line when (y1 == y2)
		//	3) Draw a line (standard method)
		// This function is not hardware-accelerated.
		HRESULT DrawLine(LONG x1, LONG y1, LONG x2, LONG y2, COLORREF dwColor, DWORD dwFlags, MGLDRAWLINEFX* pDrawLineFx);
		
		// Draws a rectangle. This function is hardware-accelerated.
		HRESULT DrawRect(RECT* pRect, COLORREF dwColor, DWORD dwFlags, MGLDRAWRECTFX *pDrawRectFx);
		
		// Fills an area (solid rectangle). This function is hardware-accelerated.
		HRESULT FillRect(RECT* pRect, COLORREF dwColor, DWORD dwFlags, MGLFILLRECTFX* pFillRectFx);
		
		// Fills an area (gradient). If no flags are specified,
		// this function draws a vertical gradient (default behavior).
		// The code that draws the gradient bands for this function
		// is hardware-accelerated.
		HRESULT FillGradient(RECT* pRect, COLORREF dwColor1, DWORD dwColor2, DWORD dwFlags, MGLFILLGRADIENTFX* pFillGradientFx);
		
		// Draws an ellipse.
		HRESULT DrawEllipse(RECT *pRect, COLORREF dwColor, DWORD dwFlags, MGLDRAWELLIPSEFX *pDrawEllipseFx);
		
		// Fills an ellipse.
		HRESULT FillEllipse(RECT *pRect, COLORREF dwColor, DWORD dwFlags, MGLFILLELLIPSEFX *pFillEllipseFx);
		
		// Draws a polyline. For performance purposes, this function
		// clips drawing to the entire area of the destination surface
		// whenever a clipper is not defined.
		HRESULT DrawPolyline(POINT *pPointArray, DWORD dwNumPoints, COLORREF dwColor, DWORD dwFlags, MGLDRAWPOLYLINEFX *pDrawPolylineFx);

		// Draws a polygon. For performance purposes, this function
		// clips drawing to the entire area of the destination surface
		// whenever a clipper is not defined.
		HRESULT DrawPolygon(POINT *pPointArray, DWORD dwNumPoints, COLORREF dwColor, DWORD dwFlags, MGLDRAWPOLYGONFX *pDrawPolygonFx);
		
		// Fills a polygon. For performance purposes, this function
		// clips drawing to the entire area of the destination surface
		// whenever a clipper is not defined.
		HRESULT FillPolygon(POINT *pPointArray, DWORD dwNumPoints, COLORREF dwColor, DWORD dwFlags, MGLFILLPOLYGONFX *pFillPolygonFx);
		
		// Draws a Bezier curve (using fixed point interpolation). For performance
		// purposes, this function clips drawing to the entire area of the destination
		// surface whenever a clipper is not defined.
		HRESULT DrawBezier(LONG startX, LONG startY,
			LONG c1x, LONG c1y,	/* control point 1 */
			LONG c2x, LONG c2y,	/* control point 2 */
			LONG endX, LONG endY, COLORREF dwColor, DWORD dwFlags, MGLDRAWBEZIER *pDrawBezierFx);
		
		// -------------------- Text Drawing ---------------------
		
		// Draws the specified text string to a surface. The ANSI version of
		// this function draws the text string using the "windows-1252" codepage.
		// (There is currently no way to change the codepage used by DrawText although
		// additional codepages may be supported by MojaveGL in the future.)
		// IMPORTANT: The ANSI version of this function treats dwStrLen
		// as the string length in bytes whereas the Unicode version treats
		// dwStrLen as the string length in WCHARs.
		HRESULT DrawText(LONG x, LONG y, const CHAR *pcszStr, DWORD dwStrLen, CMglFont *pFont, COLORREF dwColor, DWORD dwFlags, MGLDRAWTEXTFX* pDrawTextFx);
		HRESULT DrawText(LONG x, LONG y, const WCHAR *pcwszStr, DWORD dwStrLen, CMglFont *pFont, COLORREF dwColor, DWORD dwFlags, MGLDRAWTEXTFX* pDrawTextFx);
		
		// Extended version of DrawText supporting more advanced features
		// such as word wrapping.
		// IMPORTANT: The ANSI version of this function treats dwStrLen
		// as the string length in bytes whereas the Unicode version treats
		// dwStrLen as the string length in WCHARs.
		HRESULT DrawTextEx(RECT *pRect, const CHAR *pcszStr, DWORD dwStrLen, CMglFont *pFont, COLORREF dwColor, DWORD dwFlags, MGLDRAWTEXT_EX_FX* pDrawTextExFx, DWORD *pdwTextHeight /*OUT*/);
		HRESULT DrawTextEx(RECT *pRect, const WCHAR *pcwszStr, DWORD dwStrLen, CMglFont *pFont, COLORREF dwColor, DWORD dwFlags, MGLDRAWTEXT_EX_FX* pDrawTextExFx, DWORD *pdwTextHeight /*OUT*/);
		
	// Implementation
	public:
		virtual ~CMglSurface();
};

class CMglFont {

	// Construction
	public:
		CMglFont(CMgl* pMgl);
		
	// Attributes
	protected:
		CMgl* m_pMgl;
		friend class CMglSurface;
		mglinternal::MGLFONT m_font;
	
	public:
	
		// -------------------- Font Initialization ---------------------
		
		// Creates a new font from the specified data in memory.
		// The font character size is computed based on the specified DPI.
		// Pass 0 for the DPI parameters to use the default values.
		// IMPORTANT NOTE: pFontFileMem will be accessed
		// throughout the life of the font - it may not be modified or freed
		// until after the font is deleted. If you prefer to have the font object
		// manage the font data, specify the MGLFONT_LOCALCOPY flag to
		// have CreateFont() make a local copy of the font data.
		HRESULT CreateFont(DWORD dwFlags, DWORD dwStyle, DWORD dwCharWidth, DWORD dwCharHeight, DWORD dwHorizDpi, DWORD dwVertDpi, BYTE *pFontFileMem, DWORD dwFontFileSize, LONG nFaceIndex);
		
		// Frees all resources associated with the font object. This function
		// is called automatically by the destructor.
		HRESULT FreeFont();
		
		// -------------------- Font Operations ---------------------
		
		// Returns the font flags.
		DWORD GetFontFlags();
		
		// Sets the current character size. The font character size
		// is computed based on the specified DPI. Pass 0 for the
		// DPI parameters to use the default values.
		// IMPORTANT NOTE: This function is ***EXTREMELY***
		// expensive in terms of CPU usage due to the fact that it
		// recomputes font metrics for each font glyph.
		HRESULT SetCharSize(DWORD dwCharWidth, DWORD dwCharHeight, DWORD dwHorizDpi, DWORD dwVertDpi);
		
		// Returns the current font style.
		DWORD GetStyle();
		
		// Sets the current font style. The return
		// value is the previous font style.
		DWORD SetStyle(DWORD dwStyle);
		
		// Retrieves the font's text metrics.
		HRESULT GetTextMetrics(MGLTEXTMETRICS *pTextMetrics);
		
		// -------------------- String Operations ---------------------
		
		// Computes the bounding box of the specified text string.
		// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string.
		HRESULT GetStrBBox(const CHAR *pcszStr, DWORD dwStrLen, MGLBBOX *pBBox);
		HRESULT GetStrBBox(const WCHAR *pcwszStr, DWORD dwStrLen, MGLBBOX *pBBox);
		
		// Computes the width of the specified text string.
		// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string.
		DWORD GetStrWidth(const CHAR *pcszStr, DWORD dwStrLen);
		DWORD GetStrWidth(const WCHAR *pcwszStr, DWORD dwStrLen);
		
		// Gets the width of the specified line of text, taking tab characters into account. This
		// function assumes that the characters passed are left-aligned and that the first character
		// in the string is the first character drawn on a line. (In other words, this routine doesn't
		// work for characters that don't start at the beginning of a line.)
		// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string.
		DWORD GetLineWidth(const CHAR *pcszStr, DWORD dwStrLen);
		DWORD GetLineWidth(const WCHAR *pcwszStr, DWORD dwStrLen);
		
		// Gets the values needed to update a scroll bar based on a specified string
		// and the position within the string.
		HRESULT GetScrollValues(const CHAR *pcszStr, DWORD dwWidth, DWORD dwScrollPos, DWORD *pdwLines /*OUT*/, DWORD *pdwTopLine /*OUT*/);
		HRESULT GetScrollValues(const WCHAR *pcwszStr, DWORD dwWidth, DWORD dwScrollPos, DWORD *pdwLines /*OUT*/, DWORD *pdwTopLine /*OUT*/);
		
		// Determines how many bytes/characters of text can be displayed within the specified width
		// with a line break at a tab or space character. (The ANSI version returns the number of
		// bytes of text that can be displayed whereas the Unicode version returns the number of WCHARs.)
		// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string when performing the word wrap.
		DWORD WordWrap(const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth);
		DWORD WordWrap(const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth);
		
		// Word wraps a text string backwards by the number of lines specified. The character
		// position of the start of the first line and the number of lines that are actually word
		// wrapped are returned.
		HRESULT WordWrapReverseNLines(const CHAR *pcszStr, DWORD dwMaxWidth, DWORD *pdwLinesToScroll /*IN & OUT*/, DWORD *pdwScrollPos /*IN & OUT*/);
		HRESULT WordWrapReverseNLines(const WCHAR *pcwszStr, DWORD dwMaxWidth, DWORD *pdwLinesToScroll /*IN & OUT*/, DWORD *pdwScrollPos /*IN & OUT*/);
		
		// Formats text by returning a ptr to the head of a (dynamically
		// allocated) linked list, which stores break offsets for word wrapping.
		// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string.
		// IMPORTANT: Due to the overhead (and decreased performance) involved
		// in switching to ARM-native code, always call this function for word wrapping multiple
		// lines of text. If you only need to word wrap a single line of text, call WordWrap instead.
		/*
		EXAMPLE USAGE:
		{
		MGLTEXTLINENODE *pTextLineHead;
		HRESULT hr;
		
		// Format the text
		hr = m_pFont->FormatText(pcszStr, dwStrLen, dwMaxWidth, &pTextLineHead);
		if (FAILED(hr)) return hr;
		
		// Do something with 'pTextLineHead'
		
		// Free linebreak list
		m_pFont->DoneFormatText(pTextLineHead);
		}
		*/ 
		HRESULT FormatText(const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth, MGLTEXTLINENODE **ppTextLineHead);
		HRESULT FormatText(const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth, MGLTEXTLINENODE **ppTextLineHead);
		
		// Frees a (dynamically allocated) linked list, which was previously created by FormatText.
		HRESULT DoneFormatText(MGLTEXTLINENODE *pTextLineHead);
		
		// Given a pixel position, gets the offset of the character displayed at that location.
		// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string.
		HRESULT WidthToOffset(const CHAR *pcszStr, DWORD dwStrLen, DWORD dwWidth, BOOL *pbLeadingEdge /*OUT*/, DWORD *pdwTruncWidth /*OUT*/, DWORD *pdwOffset /*OUT*/);
		HRESULT WidthToOffset(const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwWidth, BOOL *pbLeadingEdge /*OUT*/, DWORD *pdwTruncWidth /*OUT*/, DWORD *pdwOffset /*OUT*/);
		
		// Finds the length of the characters from a specified string that fit within a passed width.
		// Pass MGLSTRLEN_INFINITE for the pdwStrLen value to use the entire string.
		HRESULT CharsInWidth(const CHAR *pcszStr, DWORD *pdwStrWidth /*IN & OUT*/, DWORD *pdwStrLen /*IN & OUT*/, BOOL *pbFitWithinWidth /*OUT*/);
		HRESULT CharsInWidth(const WCHAR *pcwszStr, DWORD *pdwStrWidth /*IN & OUT*/, DWORD *pdwStrLen /*IN & OUT*/, BOOL *pbFitWithinWidth /*OUT*/);
		
	// Implementation
	public:
		virtual ~CMglFont();
};

class CMglDisplay : public CMglSurface {

	// Construction
	public:
		CMglDisplay(CMgl* pMgl);
		
	// Attributes
	private:
		CMglSurface* m_pBackBuffer;
		
	// Operations
	public:
	
		// -------------------- Display Initialization ---------------------

		// Sets this display object as the primary surface and opens the display for access.
		HRESULT OpenDisplay(DWORD dwDisplayFlags);

		// Closes the display. The display is automatically closed in the CMglDisplay destructor.
		HRESULT CloseDisplay();
		
		// Returns a pointer to the back buffer of the display.
		// Note: Do not delete this pointer! It will be automatically freed by CMglDisplay.
		CMglSurface* GetBackBuffer();
		
		// Update the size of the backbuffer to the current size of the display
		HRESULT ResizeDisplay();
		
		// -------------------- Hardware Operations ---------------------
		
		// Returns the total and free amount of video memory
		HRESULT GetAvailableVidMem(DWORD dwFlags, DWORD* pTotal, DWORD* pFree);

		// Returns the hw info of the video device
		HRESULT GetHWInfo(DWORD* pInfo);
		
		// -------------------- Flipping ---------------------
		
		// Copy the back buffer to screen. Access the backbuffer using GetBackBuffer().
		HRESULT Flip();
		
	// Implementation
	public:
		virtual ~CMglDisplay();
};

#endif // __MOJAVEGL_H_