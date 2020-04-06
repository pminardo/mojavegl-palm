/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __MOJAVEGLDEFINES_H_
#define __MOJAVEGLDEFINES_H_

#include <TapWave.h>
//#include <string.h>

#ifdef _WIN32
#include <memory.h>
#endif

#define _PALM
#define _TAPWAVE
#define _HWACCEL
#define _PRGM_SECT	// compiler supports pragma mark sections

// IMPORTANT: The following constant affects how the
// MojaveGL math and image decoding functions are compiled!
#define __LITTLE_ENDIAN

/**************************************************************
* Basic Types
**************************************************************/
typedef Int32  HRESULT;
typedef UInt32 DWORD;
typedef UInt16 WORD;
typedef UInt8  BYTE;
typedef Int32  LONG;
typedef UInt32 COLORREF;
typedef float  FLOAT;
typedef double DOUBLE;
typedef Int32  LARGE_INTEGER;
typedef UInt32 SIZE_T;
typedef Int16  SHORT;
typedef char			CHAR;
typedef unsigned short	WCHAR;
#ifndef _WIN32
typedef	UInt32 size_t;
#endif

/**
 * GDI structs, macros and functions
 */
typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagRECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT, *LPRECT;

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *LPPOINT;

typedef struct tagSIZE
{
    LONG        cx;
    LONG        cy;
} SIZE, *PSIZE, *LPSIZE;

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))

#define min(a,b)							(((a)<(b))?(a):(b)) 
#define max(a,b)							(((a)>(b))?(a):(b))

#define FALSE								false
#define TRUE								true

inline // __
void SetRect(LPRECT lprc,int xLeft,int yTop,int xRight,int yBottom)
{
	if(lprc)
	{
			lprc->left = xLeft;
			lprc->top = yTop;
			lprc->right = xRight;
			lprc->bottom = yBottom; 
	}
}

inline
BOOL SetRectEmpty( LPRECT rect )
{
    if (!rect) return FALSE;
    rect->left = rect->right = rect->top = rect->bottom = 0;
    return TRUE;
}


inline // __
BOOL PtInRect(LPRECT lprc, POINT p)
{
	if (lprc)
	{
		return ((p.x >= lprc->left && p.x < lprc->right) &&
				(p.y >= lprc->top && p.y < lprc->bottom));
	}
	
	return false;
}

inline
BOOL IsRectEmpty( const RECT *rect )
{
    if (!rect) return TRUE;
    return ((rect->left >= rect->right) || (rect->top >= rect->bottom));
}

inline
BOOL IntersectRect( LPRECT dest, const RECT *src1, const RECT *src2 )
{
    if (!dest || !src1 || !src2) return FALSE;
    if (IsRectEmpty(src1) || IsRectEmpty(src2) ||
        (src1->left >= src2->right) || (src2->left >= src1->right) ||
        (src1->top >= src2->bottom) || (src2->top >= src1->bottom))
    {
        SetRectEmpty( dest );
        return FALSE;
    }
    dest->left   = max( src1->left, src2->left );
    dest->right  = min( src1->right, src2->right );
    dest->top    = max( src1->top, src2->top );
    dest->bottom = min( src1->bottom, src2->bottom );
    return TRUE;
}

/**
 * Windows status codes
 */
#define S_OK                ((HRESULT)0x00000000L)
#define E_FAIL				((HRESULT)0x80004005L)
#define E_OUTOFMEMORY       ((HRESULT)0x8007000EL)
#define E_INVALIDARG        ((HRESULT)0x80070057L)
#define SUCCEEDED(Status)   ((HRESULT)(Status) >= 0)
#define FAILED(Status)      ((HRESULT)(Status)<0)

/**
 * other
 */
#define mgl_sleep(ms)				SysTaskDelay(ms)

/* MojaveGL Memory Functions */
#define mgl_malloc(size)				Malloc(size)
#define mgl_free(p)					Free(p)
#define mgl_memset(dest, c, size)		MemSet(dest, (Int32)size, c)
#define mgl_memcpy(dest, src, size)	MemMove(dest, src, (Int32)size)

#ifndef ByteSwap16
#define ByteSwap16(n) ( ((((DWORD) n) <<8 ) & 0xFF00) | \
						((((DWORD) n) >>8 ) & 0x00FF) )
#endif

#ifndef ByteSwap32
#define ByteSwap32(n) ( ((((DWORD) n) << 24 ) & 0xFF000000) | \
						((((DWORD) n) << 8 ) & 0x00FF0000) | \
						((((DWORD) n) >> 8 ) & 0x0000FF00) | \
						((((DWORD) n) >> 24 ) & 0x000000FF) )
#endif

#ifndef abs
#define abs(a)	(((a) < 0) ? -(a) : (a))
#endif

/**************************************************************
* MojaveGL Internals
**************************************************************/

/* Constants used internally by the MojaveGL blitters */
#define OPACITY_0	0
#define OPACITY_50	128
#define OPACITY_100	255

// Blit helper: Use Duff's device to explicily loop unroll.
// If you don't know how to use this, take a look at:
// http://www.lysator.liu.se/c/duffs-device.html.
// Typical usage would be (in your blit loop, of course!):
// 	count = rowPixels, rowBytes, or yPitch
//	expr = { *pDest++ = *pSrc++; }
#define BLITTER_DUFF_DEVICE(count, expr)\
	{ \
	register LONG _dd_n = (((LONG)count)+7)/8;\
	switch(count%8){\
		case 0:	do{	expr \
		case 7:		expr \
		case 6:		expr \
		case 5:		expr \
		case 4:		expr \
		case 3:		expr \
		case 2:		expr \
		case 1:		expr \
			}while(--_dd_n>0); \
		} \
	}
	
// Same as above but with 16x loop unrolling
#define BLITTER_DUFF_DEVICE_16x(count, expr)\
	{ \
	register LONG _dd_n = (((LONG)count)+15) >> 4;\
	switch(count&15){\
		case 0:	do{	expr \
		case 15:		expr \
		case 14:		expr \
		case 13:		expr \
		case 12:		expr \
		case 11:		expr \
		case 10:		expr \
		case 9:		expr \
		case 8:		expr \
		case 7:		expr \
		case 6:		expr \
		case 5:		expr \
		case 4:		expr \
		case 3:		expr \
		case 2:		expr \
		case 1:		expr \
			}while(--_dd_n>0); \
		} \
	}

#endif // __MOJAVEGLDEFINES_H_
