/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftbitmap.h>
#include <freetype/internal/ftobjs.h>

#include "../string/S_string.h"
#include "../blitter/BL_pixelutil.h"
#include "../blitter/BL_drawftbmp1.h"
#include "../blitter/BL_drawftbmp8.h"
#include "../blitter/BL_fillrect.h"

#include "FT_defines.h"
#include "FT_font.h"
#include "FT_glyphcache.h"

/**************************************************************
* Constants
**************************************************************/

#define NULL_CHAR			0x0000
#define HORIZ_TAB_CHAR		0x0009
#define LINEFEED_CHAR		0x000A
#define SPACE_CHAR			0x0020

#define TAB_CHAR_WIDTH 		20

/**************************************************************
* Internal Structures
**************************************************************/

// All-purpose glyph data structure, used by FT_GetCharGlyph().
typedef struct _GLYPHDATA {

	DWORD dwWidth;
	DWORD dwHeight;
	LONG nLeft;
	LONG nTop;
	
	LONG xAdvance;
	LONG yAdvance;
	
	BYTE pixelMode;	// monochrome or gray
	WORD maxGrays;	// maximum gray level value (in the range 1 to 255)

	LONG pitch;		// bytes per bitmap line; may be positive or negative
	void *pBuffer;		// bitmap pixels

} GLYPHDATA;

/**************************************************************
* Helper Functions
**************************************************************/

namespace FTHelper {

	inline
	void ComputeItalicMatrix(FT_Face ftf, FT_Matrix *pftMatrix /*OUT*/)
	{
		DWORD dwHeight;
		
		dwHeight = ftf->size->metrics.height >> 6;
		
		// Build the ITALIC typeface matrix
		pftMatrix->xx = 1<<16;
		pftMatrix->xy =  (int)(((double)0.207F*dwHeight)*(1<<16))/dwHeight;
		pftMatrix->yx = 0;
		pftMatrix->yy = 1<<16;
	}
	
	inline
	void ComputeItalicAdjust(FT_Pos ftCharWidth /*26.6 fixed point*/, FT_Pos *pftXAdjust /*OUT*/)
	{
		// This seems to be the fastest ITALIC adjustment computation
		*pftXAdjust = (ftCharWidth >> 4)-(ftCharWidth >> 5);
	}

	inline
	void ComputeBoldStrength(FT_Pos ftCharWidth /*26.6 fixed point*/, FT_Pos ftCharHeight /*26.6 fixed point*/,
		FT_Pos *pftXStrength /*OUT*/, FT_Pos *pftYStrength /*OUT*/)
	{
		*pftXStrength = ftCharWidth/8;
		*pftYStrength = ftCharHeight/8;
	}
	
	// Compute adjustment deltas to make a glyph's width & height (extent)
	// reflect that of a "bold" character.
	inline
	FT_Error ComputeBoldExtentAdjust(FT_Pos ftXStrength /*26.6 fixed point*/, FT_Pos ftYStrength /*26.6 fixed point*/,
		LONG *pXAdjust /*OUT*/, LONG *pYAdjust /*OUT*/)
	{
		ftXStrength = FT_PIX_ROUND( ftXStrength ) >> 6;
		ftYStrength = FT_PIX_ROUND( ftYStrength ) >> 6;
 		
 		// Check for error
	 	if (ftXStrength == 0 && ftYStrength == 0)
			return FT_Err_Ok;
		else if (ftXStrength < 0 || ftYStrength < 0)
			return FT_Err_Invalid_Argument;
			
		// Return values
		*pXAdjust = (LONG)ftXStrength;
		*pYAdjust = (LONG)ftYStrength;
		
		return FT_Err_Ok;
	}
	
	inline
	void ComputeBoldAdjust(FT_Pos ftCharWidth /*26.6 fixed point*/, FT_Pos *pftXAdjust /*OUT*/)
	{
		*pftXAdjust = (ftCharWidth >> 4)+(ftCharWidth >> 5);
	}

}

namespace DrawText565 {

	inline
	void DrawFTBmp1(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, LONG destX, LONG destY,
		void *src, LONG srcPitch, LONG srcWidth, LONG srcHeight, RECT *pClipRect,
		DWORD dwNativeColor, BOOL bUseOpacity, DWORD dwOpacity)
	{
		RECT srcRect;
		LONG destRight, destBottom, srcRectWidth, srcRectHeight;
		
		// Init the source rectangle
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcWidth;
		srcRect.bottom = srcHeight;
		
		/* Now handle negative values in the rectangles.
		    Handle the case where nothing is to be done.
		*/
		if ((destX   >= destWidth) ||
			(destY    >= destHeight) ||
			(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
			(srcRect.top >= srcHeight) ||
			(srcRect.left >= srcWidth))
		{
			return;
		}
		
		destRight = destX+srcWidth/*srcRect.right-srcRect.left*/;
		destBottom = destY+srcHeight/*srcRect.bottom-srcRect.top*/;
		
		// ------------------------------------------
		// Clipping
		// ------------------------------------------
		if (IsRectEmpty(pClipRect))
		{
			if (destX < 0 || destY < 0 ||
				destRight > destWidth ||
				destBottom > destHeight)
			{
				return;
			}
		}
		else
		{
			// Handle clipping (left)
			if (destX < pClipRect->left)
			{
				srcRect.left += pClipRect->left-destX;
				destX = pClipRect->left;
			}
			
			// Handle clipping (right)
			if (destRight > pClipRect->right)
			{
				srcRect.right -= destRight - pClipRect->right;
			}
			
			// Handle clipping (top)
			if (destY < pClipRect->top)
			{
				srcRect.top += pClipRect->top-destY;
				destY = pClipRect->top;
			}
			
			// Handle clipping (bottom)
			if (destBottom > pClipRect->bottom)
			{
				srcRect.bottom -= destBottom - pClipRect->bottom;
			}
			
			// Make sure that there's something to blit
			if (IsRectEmpty(&srcRect))
				return;
		}

		srcRectWidth = srcRect.right - srcRect.left;
		srcRectHeight = srcRect.bottom - srcRect.top;
		
		// ------------------------------------------
		// Drawing
		// ------------------------------------------
		if (bUseOpacity)
		{
			BL_DrawFTBmp1Opacity_565(dest, src, destXPitch, destYPitch,
				srcPitch, destX, destY, destWidth, destHeight,
				srcRect.left, srcRect.top, srcRectWidth, srcRectHeight, dwNativeColor, dwOpacity);
		}
		else
		{
			BL_DrawFTBmp1_565(dest, src, destXPitch, destYPitch,
				srcPitch, destX, destY, destWidth, destHeight,
				srcRect.left, srcRect.top, srcRectWidth, srcRectHeight, dwNativeColor);
		}
	}

	inline
	void DrawFTBmp8(void *dest, LONG destXPitch, LONG destYPitch,
		LONG destWidth, LONG destHeight, LONG destX, LONG destY,
		void *src, LONG srcPitch, LONG srcWidth, LONG srcHeight, RECT *pClipRect,
		DWORD dwNativeColor, BOOL bUseOpacity, DWORD dwOpacity)
	{
		RECT srcRect;
		LONG destRight, destBottom, srcRectWidth, srcRectHeight;
		
		// Init the source rectangle
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = srcWidth;
		srcRect.bottom = srcHeight;
		
		/* Now handle negative values in the rectangles.
		    Handle the case where nothing is to be done.
		*/
		if ((destX   >= destWidth) ||
			(destY    >= destHeight) ||
			(srcRect.bottom <= 0) || (srcRect.right <= 0)     ||
			(srcRect.top >= srcHeight) ||
			(srcRect.left >= srcWidth))
		{
			return;
		}
		
		destRight = destX+srcWidth/*srcRect.right-srcRect.left*/;
		destBottom = destY+srcHeight/*srcRect.bottom-srcRect.top*/;
		
		// ------------------------------------------
		// Clipping
		// ------------------------------------------
		if (IsRectEmpty(pClipRect))
		{
			if (destX < 0 || destY < 0 ||
				destRight > destWidth ||
				destBottom > destHeight)
			{
				return;
			}
		}
		else
		{
			// Handle clipping (left)
			if (destX < pClipRect->left)
			{
				srcRect.left += pClipRect->left-destX;
				destX = pClipRect->left;
			}
			
			// Handle clipping (right)
			if (destRight > pClipRect->right)
			{
				srcRect.right -= destRight - pClipRect->right;
			}
			
			// Handle clipping (top)
			if (destY < pClipRect->top)
			{
				srcRect.top += pClipRect->top-destY;
				destY = pClipRect->top;
			}
			
			// Handle clipping (bottom)
			if (destBottom > pClipRect->bottom)
			{
				srcRect.bottom -= destBottom - pClipRect->bottom;
			}
			
			// Make sure that there's something to blit
			if (IsRectEmpty(&srcRect))
				return;
		}

		srcRectWidth = srcRect.right - srcRect.left;
		srcRectHeight = srcRect.bottom - srcRect.top;
		
		// ------------------------------------------
		// Drawing
		// ------------------------------------------
		if (bUseOpacity)
		{
			BL_DrawFTBmp8Opacity_565(dest, src, destXPitch, destYPitch,
				srcPitch, destX, destY, destWidth, destHeight,
				srcRect.left, srcRect.top, srcRectWidth, srcRectHeight, dwNativeColor, dwOpacity);
		}
		else
		{
			BL_DrawFTBmp8_565(dest, src, destXPitch, destYPitch,
				srcPitch, destX, destY, destWidth, destHeight,
				srcRect.left, srcRect.top, srcRectWidth, srcRectHeight, dwNativeColor);
		}
	}

}

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ Font Functions --------------
#endif

// Initialize the FreeType library.
// Returns: FreeType error code.
FTERR FT_OpenLibrary(FTLIBRARY *ftLibrary /*OUT*/)
{
	FT_Error ftError;
	FT_Library ftlib;
	
	// Verify parameters
	if (!ftLibrary) return FT_Err_Invalid_Argument;
	
	// Init FreeType library
	ftError = FT_Init_FreeType (&ftlib);
	if (ftError != FT_Err_Ok) return ftError;
	
	// Return FreeType library instance
	*ftLibrary = (FTLIBRARY)ftlib;
	
	return FT_Err_Ok;
}

// Close the FreeType library instance.
// Returns: FreeType error code.
FTERR FT_CloseLibrary(FTLIBRARY ftLibrary)
{
	FT_Error ftError;
	
	// Verify parameters
	if (!ftLibrary) return FT_Err_Invalid_Argument;
	
	// Close FreeType library
	ftError = FT_Done_FreeType ((FT_Library)ftLibrary);
	if (ftError != FT_Err_Ok) return ftError;

	return FT_Err_Ok;
}

// Create a font face from a memory buffer.
// Returns: FreeType error code.
FTERR FT_CreateFontFaceMemory(FTLIBRARY ftLibrary, const BYTE *pFontData, DWORD dwFontDataSize, LONG nFaceIndex,
	DWORD dwCharWidth, DWORD dwCharHeight, DWORD dwHorizDpi, DWORD dwVertDpi,
	FTFACE *ftFace/*OUT*/, FTGLYPHCACHE *ftGlyphCache/*OUT*/, DWORD dwGlyphCacheMaxItems, LONG *pnAscent /*OUT*/, LONG *pnDescent/*OUT*/, LONG *pnHeight/*OUT*/, LONG *pnMaxHorizAdvance/*OUT*/)
{
	FT_Error ftError;
	FT_Face ftf;
	FT_CacheHandle ftc;
	FT_F26Dot6 ftCharWidth, ftCharHeight;
	
	// Verify parameters
	if (!ftLibrary || !pFontData || dwFontDataSize == 0 || !ftFace)
		return FT_Err_Invalid_Argument;
		
	// Create the cache, if needed
	if (ftGlyphCache)
	{
		ftError = FT_CreateGlyphCache(dwGlyphCacheMaxItems, &ftc);
		if (ftError != FT_Err_Ok) return ftError;
		
		// Return the cache handle
		*ftGlyphCache = ftc;
	}
	
	// Create the font face
	ftError = FT_New_Memory_Face((FT_Library)ftLibrary, pFontData, (FT_Long)dwFontDataSize, nFaceIndex, &ftf);
	if (ftError != FT_Err_Ok)
	{
		// Delete the glyph cache, if created
		if (ftGlyphCache) FT_DeleteGlyphCache(ftc);
		
		return ftError;
	}
	
	// Convert char width & height to 26.6 fixed point
	ftCharWidth = (FT_F26Dot6)dwCharWidth << 6;
	ftCharHeight = (FT_F26Dot6)dwCharHeight << 6;
	
	// Set the font face character size
	ftError = FT_Set_Char_Size(ftf, ftCharWidth, ftCharHeight, dwHorizDpi, dwVertDpi);
	if (ftError != FT_Err_Ok)
	{
		// Delete the glyph cache, if created
		if (ftGlyphCache) FT_DeleteGlyphCache(ftc);
	
		// Delete the font face
		FT_Done_Face(ftf);
		
		return ftError;
	}
	
	// Return font face
	*ftFace = (FTFACE)ftf;
	
	// Return font metrics, if requested
	if (pnAscent) *pnAscent = ftf->size->metrics.ascender >> 6;
	if (pnDescent) *pnDescent = ftf->size->metrics.descender >> 6;
	if (pnHeight) *pnHeight = ftf->size->metrics.height >> 6;
	if (pnMaxHorizAdvance) *pnMaxHorizAdvance = ftf->size->metrics.max_advance >> 6;

	return FT_Err_Ok;
}

// Set the font face character size.
// Returns: FreeType error code.
FTERR FT_SetFontFaceCharSize(FTFACE ftFace, DWORD dwCharWidth, DWORD dwCharHeight, DWORD dwHorizDpi, DWORD dwVertDpi,
	LONG *pnAscent /*OUT*/, LONG *pnDescent/*OUT*/, LONG *pnHeight/*OUT*/, LONG *pnMaxHorizAdvance/*OUT*/)
{
	FT_Error ftError;
	FT_Face ftf;
	FT_F26Dot6 ftCharWidth, ftCharHeight;
	
	// Verify parameters
	if (!ftFace) return FT_Err_Invalid_Argument;
	
	ftf = (FT_Face)ftFace;
	
	// Convert char width & height to 26.6 fixed point
	ftCharWidth = (FT_F26Dot6)dwCharWidth << 6;
	ftCharHeight = (FT_F26Dot6)dwCharHeight << 6;
	 
	// Set the font face character size
	ftError = FT_Set_Char_Size(ftf, ftCharWidth, ftCharHeight, dwHorizDpi, dwVertDpi);
	if (ftError != FT_Err_Ok) return ftError;
	
	// Return font metrics, if requested
	if (pnAscent) *pnAscent = ftf->size->metrics.ascender >> 6;
	if (pnDescent) *pnDescent = ftf->size->metrics.descender >> 6;
	if (pnHeight) *pnHeight = ftf->size->metrics.height >> 6;
	if (pnMaxHorizAdvance) *pnMaxHorizAdvance = ftf->size->metrics.max_advance >> 6;

	return FT_Err_Ok;
}

// Retrieves text metrics for the specified font face.
void FT_GetTextMetrics(FTFACE ftFace, LONG *pnAscent /*OUT*/, LONG *pnDescent/*OUT*/,
	LONG *pnHeight/*OUT*/, LONG *pnMaxHorizAdvance/*OUT*/)
{
	FT_Face ftf = (FT_Face)ftFace;
	
	if (pnAscent) *pnAscent = ftf->size->metrics.ascender >> 6;
	if (pnDescent) *pnDescent = ftf->size->metrics.descender >> 6;
	if (pnHeight) *pnHeight = ftf->size->metrics.height >> 6;
	if (pnMaxHorizAdvance) *pnMaxHorizAdvance = ftf->size->metrics.max_advance >> 6;
}

// Retrieves the width (+ kerning, if applicable) of the specified character.
// Returns: TRUE if successful; otherwise, FALSE if an error occurred.
inline
BOOL _FT_GetCharWidth(FT_Face ftf, DWORD dwFontStyleFlags,
	FT_ULong ch, FT_Int32 ftLoadGlyphFlags, FT_Bool ftUseKerning,
	FT_UInt ftPreviousGlyphIndex, DWORD *pdwCharWidth /*OUT*/, FT_UInt *pftGlyphIndex /*OUT*/)
{
	FT_Error ftError;
	FT_UInt ftGlyphIndex;
	LONG nGlyphWidth;
	FT_GlyphSlot ftSlot;
	DWORD dwWidth;
	
	ftSlot = ftf->glyph; /* a small shortcut */
	dwWidth = 0;

	// Retrieve glyph index from character code
	ftGlyphIndex = FT_Get_Char_Index(ftf, ch);
	
	// ------------------------
	// Load the glyph
	// ------------------------
	ftError = FT_Load_Glyph(ftf, ftGlyphIndex, ftLoadGlyphFlags);
	if (ftError != FT_Err_Ok) goto error;	// ignore errors
	
	// Retrieve kerning distance and move pen position
	if (ftUseKerning && ftPreviousGlyphIndex && ftGlyphIndex)
	{
		FT_Vector ftDelta;
		
		FT_Get_Kerning(ftf, ftPreviousGlyphIndex, ftGlyphIndex, FT_KERNING_DEFAULT, &ftDelta );
		dwWidth += ftDelta.x >> 6;
	}
	
	// -----------------------------------------
	// Adjust for ITALIC typeface
	// -----------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Pos ftXAdjust;
		
		FTHelper::ComputeItalicAdjust(ftSlot->advance.x /*26.6 fixed point*/, &ftXAdjust);
		ftSlot->advance.x += ftXAdjust;
	}
	
	// Get glyph width (convert from 26.6 fixed point)
	nGlyphWidth = ftSlot->advance.x >> 6;
	
	// -----------------------------------------
	// Adjust for BOLD typeface
	// -----------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_BOLD)
	{
		FT_Pos ftXStrength, ftYStrength;
		LONG xAdjust, yAdjust;
		
		FTHelper::ComputeBoldStrength(ftSlot->advance.x, ftSlot->advance.y, &ftXStrength, &ftYStrength);
		ftError = FTHelper::ComputeBoldExtentAdjust(ftXStrength, ftYStrength, &xAdjust, &yAdjust);
		
		if (ftError != FT_Err_Ok) goto error;	// ignore errors
		
		// Adjust glyph width
		nGlyphWidth += xAdjust;
	}
	
	// Adjust width
	dwWidth += nGlyphWidth;
	
	// ----------------------------------
	// Return values
	// ----------------------------------
	
	*pdwCharWidth = dwWidth;
	
	if (pftGlyphIndex)
		*pftGlyphIndex = ftGlyphIndex;
	
	return TRUE;
	
	error:
	
	return FALSE;
}

void FT_ComputeBBoxA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD *pdwWidth /*OUT*/, DWORD *pdwHeight /*OUT*/)
{
	CHAR *p;
	WCHAR rgwStr[2];
	LONG srclen;
	DWORD dwCharWidth;
	DWORD dwWidth, dwHeight;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (CHAR*)pcszStr;
	dwWidth = 0;
	dwHeight = ftf->size->metrics.height >> 6;
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (*p && dwStrLen)
	{
		// Determine the length of the character that
		// we need to convert to Unicode.
		srclen = (S_IsDBCSLeadByte(*p) && *(p+1)) ? 2 : 1;
	
		// Convert the current character to Unicode.
		// If an error occurred, just skip to the next character.
		if (S_MultiByteToWideChar(CP_ACP, 0, p, srclen, rgwStr, sizeof(rgwStr)) == 0)
			goto nextChar;
			
		// ---------------------------------------------------
		// Retrieve width of the current character
		// ----------------------------------------------------
		if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*rgwStr,
			ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
			&dwCharWidth, &ftPreviousGlyphIndex))
		{
			// An error occurred. Just advance to the next character.
			goto nextChar;
		}
		
		// Adjust current width
		dwWidth += dwCharWidth;
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		p += srclen; /*S_CharNextA(p);*/
		dwStrLen -= srclen;
	}
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
	
	// Return requested variables
	if (pdwWidth) *pdwWidth = dwWidth;
	if (pdwHeight) *pdwHeight = dwHeight;
}

void FT_ComputeBBoxW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD *pdwWidth /*OUT*/, DWORD *pdwHeight /*OUT*/)
{
	WCHAR *p;
	DWORD dwCharWidth;
	DWORD dwWidth, dwHeight;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (WCHAR*)pcwszStr;
	dwWidth = 0;
	dwHeight = ftf->size->metrics.height >> 6;
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (*p && dwStrLen)
	{
		// ---------------------------------------------------
		// Retrieve width of the current character
		// ----------------------------------------------------
		if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*p,
			ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
			&dwCharWidth, &ftPreviousGlyphIndex))
		{
			// An error occurred. Just advance to the next character.
			goto nextChar;
		}
		
		// Adjust current width
		dwWidth += dwCharWidth;
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		p = S_CharNextW(p);
		--dwStrLen;
	}
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
	
	// Return requested variables
	if (pdwWidth) *pdwWidth = dwWidth;
	if (pdwHeight) *pdwHeight = dwHeight;
}

void FT_ComputeLineBBoxA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD *pdwWidth /*OUT*/, DWORD *pdwHeight /*OUT*/)
{
	CHAR *p;
	WCHAR rgwStr[2];
	LONG srclen;
	DWORD dwCharWidth;
	DWORD dwWidth, dwHeight;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (CHAR*)pcszStr;
	dwWidth = 0;
	dwHeight = ftf->size->metrics.height >> 6;
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (*p && dwStrLen)
	{
		// Determine the length of the character that
		// we need to convert to Unicode.
		srclen = (S_IsDBCSLeadByte(*p) && *(p+1)) ? 2 : 1;
	
		// Convert the current character to Unicode.
		// If an error occurred, just skip to the next character.
		if (S_MultiByteToWideChar(CP_ACP, 0, p, srclen, rgwStr, sizeof(rgwStr)) == 0)
			goto nextChar;
	
		switch (*rgwStr)
		{
			case HORIZ_TAB_CHAR:
			
			// Adjust width
			dwWidth += TAB_CHAR_WIDTH - (dwWidth % TAB_CHAR_WIDTH);
			
			break;
		
			default:
			
			// ---------------------------------------------------
			// Retrieve width of the current character
			// ----------------------------------------------------
			if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*rgwStr,
				ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
				&dwCharWidth, &ftPreviousGlyphIndex))
			{
				// An error occurred. Just advance to the next character.
				goto nextChar;
			}
			
			// Adjust current width
			dwWidth += dwCharWidth;
			
			break;
		}
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		p += srclen; /*S_CharNextA(p);*/
		dwStrLen -= srclen;
	}
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
	
	// Return requested variables
	if (pdwWidth) *pdwWidth = dwWidth;
	if (pdwHeight) *pdwHeight = dwHeight;
}

void FT_ComputeLineBBoxW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD *pdwWidth /*OUT*/, DWORD *pdwHeight /*OUT*/)
{
	WCHAR *p;
	DWORD dwCharWidth;
	DWORD dwWidth, dwHeight;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (WCHAR*)pcwszStr;
	dwWidth = 0;
	dwHeight = ftf->size->metrics.height >> 6;
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (*p && dwStrLen)
	{
		switch (*p)
		{
			case HORIZ_TAB_CHAR:
			
			// Adjust width
			dwWidth += TAB_CHAR_WIDTH - (dwWidth % TAB_CHAR_WIDTH);
			
			break;
		
			default:
			
			// ---------------------------------------------------
			// Retrieve width of the current character
			// ----------------------------------------------------
			if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*p,
				ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
				&dwCharWidth, &ftPreviousGlyphIndex))
			{
				// An error occurred. Just advance to the next character.
				goto nextChar;
			}
			
			// Adjust current width
			dwWidth += dwCharWidth;
			
			break;
		}
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		p = S_CharNextW(p);
		--dwStrLen;
	}
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
	
	// Return requested variables
	if (pdwWidth) *pdwWidth = dwWidth;
	if (pdwHeight) *pdwHeight = dwHeight;
}

DWORD FT_WordWrapA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth)
{
	const CHAR rgcWrapBreakCharTable[] = {
		HORIZ_TAB_CHAR,
		LINEFEED_CHAR, /* linefeed char */
		SPACE_CHAR, /* space char */
		'\0' /* denotes ending of wordwrap char table */};

	CHAR *p, *pnew, *pcurr;
	WCHAR rgwStr[2];
	LONG srclen;
	DWORD dwCharWidth;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	DWORD dwOffset, dwWidth, i, dwPos, dwLineWidth;
	BOOL bBreakChrFound;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (CHAR*)pcszStr;
	dwOffset = 0;
	dwWidth = 0;
	dwLineWidth = 0; // count of characters
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (dwWidth < dwMaxWidth)
	{
		// -------------------------------------------------
		// If we've exceeded the maxWidth then stop.
		// -------------------------------------------------
		if (dwOffset >= dwStrLen)
		{
			dwLineWidth = dwOffset;
			goto done;
		}
		
		// A null (end of string) immediately terminates processing, and
		// the offset is before (to the left of) the null byte.
		if (*p == NULL_CHAR)
		{
			dwLineWidth = dwOffset;
			goto done;
		}
		
		// ----------------------------------------
		
		// Determine the length of the character that
		// we need to convert to Unicode.
		srclen = (S_IsDBCSLeadByte(*p) && *(p+1)) ? 2 : 1;
	
		// Convert the current character to Unicode.
		// If an error occurred, just skip to the next character.
		if (S_MultiByteToWideChar(CP_ACP, 0, p, srclen, rgwStr, sizeof(rgwStr)) == 0)
			goto nextChar;
			
		// ----------------------------------------
		
		switch (*rgwStr)
		{
			// A linefeed immediately terminates processing, and the offset is
			// to the right of the linefeed character.
			case LINEFEED_CHAR:
				dwLineWidth = dwOffset+srclen;
				goto done;
		
			// A tab's width is the distance from the current line width to the
			// next tab-stop position (currently fixed at TAB_CHAR_WIDTH pixels).
			case HORIZ_TAB_CHAR:
				dwWidth += TAB_CHAR_WIDTH - (dwWidth % TAB_CHAR_WIDTH);
				break;
		
			default:
			
				// ---------------------------------------------------
				// Retrieve width of the current character
				// ----------------------------------------------------
				if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*rgwStr,
					ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
					&dwCharWidth, &ftPreviousGlyphIndex))
				{
					// An error occurred. Just advance to the next character.
					goto nextChar;
				}
				
				// Adjust current width
				dwWidth += dwCharWidth;
				
				break;
		}
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		dwOffset += srclen;
		p += srclen;
	}
	
	// Figure out the break offset. If not even one character fit, then take
	// the last character anyway.
	if (dwWidth > dwMaxWidth)
	{
		//--dwOffset;
	
		// Compute the offset between the current dwOffset
		// and the "previous" one.
		pcurr = (CHAR*)&pcszStr[dwOffset];
		pnew = S_CharPrevA(pcszStr, pcurr);
		dwOffset -= pcurr - pnew;
		
		if (dwOffset == 0)
		{
			//dwOffset = 1;
			dwOffset = (S_IsDBCSLeadByte(pcszStr[dwOffset]) && pcszStr[dwOffset+1]) ? 2 : 1;
		}
	}
	
	// If we're at the end of the string, then that's the break location.
	if (!pcszStr[dwOffset])
	{
		dwLineWidth = dwOffset;
		goto done;
	}
	
	// First walk the offset right by one character, since we'll be moving
	// through the text right to left, and we want to start with the first
	// character after dwOffset.
	
	// Run backwards, looking for the first break character (added by Parker)
	dwPos = dwOffset;
	bBreakChrFound = FALSE;
	while (dwPos && !bBreakChrFound)
	{
		// Check the current character against the break table.
		i = 0;
		while (rgcWrapBreakCharTable[i])
		{
			if (rgcWrapBreakCharTable[i] == pcszStr[dwPos])
			{
				bBreakChrFound = TRUE;
				dwOffset = dwPos;
				break;
			}
		
			++i;
		}
		
		// --dwPos;
	
		// Compute the offset between the current dwPos
		// and the "previous" one.
		pcurr = (CHAR*)&pcszStr[dwPos];
		pnew = S_CharPrevA(pcszStr, pcurr);
		dwPos -= pcurr - pnew;
	}
	
	// Grab a following tab
	if (pcszStr[dwOffset] == HORIZ_TAB_CHAR /* tab character */) {
		++dwOffset;
	}
	
	// Grab all of the following spaces
	while (pcszStr[dwOffset] == SPACE_CHAR /* space character */) {
		++dwOffset;
	}
	
	// Grab a following linefeed
	if (pcszStr[dwOffset] == LINEFEED_CHAR /* linefeed*/) {
		++dwOffset;
	}
	
	dwLineWidth = dwOffset;
	
	done:
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
	
	return dwLineWidth;
}

DWORD FT_WordWrapW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth)
{
	const WCHAR rgcwWrapBreakCharTable[] = {
		HORIZ_TAB_CHAR,
		LINEFEED_CHAR, /* linefeed char */
		SPACE_CHAR, /* space char */
		'\0' /* denotes ending of wordwrap char table */};

	WCHAR *p, *pnew;
	DWORD dwCharWidth;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	DWORD dwOffset, dwWidth, i, dwPos, dwLineWidth;
	BOOL bBreakChrFound;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (WCHAR*)pcwszStr;
	dwOffset = 0;
	dwWidth = 0;
	dwLineWidth = 0; // count of characters
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (dwWidth < dwMaxWidth)
	{
		// -------------------------------------------------
		// If we've exceeded the maxWidth then stop.
		// -------------------------------------------------
		if (dwOffset >= dwStrLen)
		{
			dwLineWidth = dwOffset;
			goto done;
		}
		
		switch (*p)
		{
			// A null (end of string) immediately terminates processing, and
			// the offset is before (to the left of) the null byte.
			case NULL_CHAR:
				dwLineWidth = dwOffset;
				goto done;
				
			// A linefeed immediately terminates processing, and the offset is
			// to the right of the linefeed character.
			case LINEFEED_CHAR:
				dwLineWidth = dwOffset+1;
				goto done;
		
			// A tab's width is the distance from the current line width to the
			// next tab-stop position (currently fixed at TAB_CHAR_WIDTH pixels).
			case HORIZ_TAB_CHAR:
				dwWidth += TAB_CHAR_WIDTH - (dwWidth % TAB_CHAR_WIDTH);
				break;
		
			default:
			
				// ---------------------------------------------------
				// Retrieve width of the current character
				// ----------------------------------------------------
				if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*p,
					ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
					&dwCharWidth, &ftPreviousGlyphIndex))
				{
					// An error occurred. Just advance to the next character.
					goto nextChar;
				}
				
				// Adjust current width
				dwWidth += dwCharWidth;
				
				break;
		}
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		pnew = S_CharNextW(p);
		dwOffset += pnew - p;
		p = pnew;
	}
	
	// Figure out the break offset. If not even one character fit, then take
	// the last character anyway.
	if (dwWidth > dwMaxWidth)
	{
		// Compute the offset between the current dwOffset
		// and the "previous" one.
		//pcurr = (WCHAR*)&pcwszStr[dwOffset];
		//pnew = S_CharPrevW(pcwszStr, pcurr);
		
		--dwOffset;
		//dwOffset -= pcurr - pnew;
		
		if (dwOffset == 0)
		{
			dwOffset = 1;
			//dwOffset = (S_IsDBCSLeadByte(&pcszStr[dwOffset])) ? 2 : 1;
		}
	}
	
	// If we're at the end of the string, then that's the break location.
	if (!pcwszStr[dwOffset])
	{
		dwLineWidth = dwOffset;
		goto done;
	}
	
	// First walk the offset right by one character, since we'll be moving
	// through the text right to left, and we want to start with the first
	// character after dwOffset.
	
	// Run backwards, looking for the first break character (added by Parker)
	dwPos = dwOffset;
	bBreakChrFound = FALSE;
	while (dwPos && !bBreakChrFound)
	{
		// Check the current character against the break table.
		i = 0;
		while (rgcwWrapBreakCharTable[i])
		{
			if (rgcwWrapBreakCharTable[i] == pcwszStr[dwPos])
			{
				bBreakChrFound = TRUE;
				dwOffset = dwPos;
				break;
			}
		
			++i;
		}
	
		// Compute the offset between the current dwOffset
		// and the "previous" one.
		//pcurr = (WCHAR*)&pcwszStr[dwOffset];
		//pnew = S_CharPrevW(pcwszStr, pcurr);
		//dwPos -= pcurr - pnew;
		--dwPos;
	}
	
	// Grab a following tab
	if (pcwszStr[dwOffset] == HORIZ_TAB_CHAR /* tab character */) {
		++dwOffset;
	}
	
	// Grab all of the following spaces
	while (pcwszStr[dwOffset] == SPACE_CHAR /* space character */) {
		++dwOffset;
	}
	
	// Grab a following linefeed
	if (pcwszStr[dwOffset] == LINEFEED_CHAR /* linefeed*/) {
		++dwOffset;
	}
	
	dwLineWidth = dwOffset;
	
	done:
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
	
	return dwLineWidth;
}

void FT_GetScrollValuesA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwWidth, DWORD dwScrollPos,
	DWORD *pdwLines /*OUT*/, DWORD *pdwTopLine /*OUT*/)
{
	DWORD dwLength, dwLines, dwTopLine;
	
	dwLength = 0;
	dwLines = 0;
	dwTopLine = 0;
	
	do {
		dwLength = FT_WordWrapA(ftFace, dwFontStyleFlags,
			&pcszStr[dwLength], MGLSTRLEN_INFINITE, dwWidth);
		
		++dwLines;
		
		if (dwLength <= dwScrollPos)
			dwTopLine = dwLines;
	
	} while (pcszStr[dwLength]);
	
	// If the text ends with a linefeed, add one to the height.
	if (dwLength && pcszStr[dwLength-1] == LINEFEED_CHAR)
		++dwLines;
		
	// Return values
	*pdwLines = dwLines;
	*pdwTopLine = dwTopLine;
}

void FT_GetScrollValuesW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwWidth, DWORD dwScrollPos,
	DWORD *pdwLines /*OUT*/, DWORD *pdwTopLine /*OUT*/)
{
	DWORD dwLength, dwLines, dwTopLine;
	
	dwLength = 0;
	dwLines = 0;
	dwTopLine = 0;
	
	do {
		dwLength = FT_WordWrapW(ftFace, dwFontStyleFlags,
			&pcwszStr[dwLength], MGLSTRLEN_INFINITE, dwWidth);
		
		++dwLines;
		
		if (dwLength <= dwScrollPos)
			dwTopLine = dwLines;
	
	} while (pcwszStr[dwLength]);
	
	// If the text ends with a linefeed, add one to the height.
	if (dwLength && pcwszStr[dwLength-1] == LINEFEED_CHAR)
		++dwLines;
		
	// Return values
	*pdwLines = dwLines;
	*pdwTopLine = dwTopLine;
}

static BOOL _FT_FindReverseCharA(const CHAR *pStr, CHAR charToFind, DWORD dwLen, DWORD *pdwPos)
{
	const CHAR *p;
	
	p = pStr+dwLen-1;
	
	while (dwLen)
	{
		if (*p == charToFind)
		{
			*pdwPos = dwLen-1;
			return TRUE;
		}
		
		--p;
		--dwLen;
	}
	
	return FALSE;
}

static BOOL _FT_FindReverseCharW(const WCHAR *pStr, WCHAR charToFind, DWORD dwLen, DWORD *pdwPos)
{
	const WCHAR *p;
	
	p = pStr+dwLen-1;
	
	while (dwLen)
	{
		if (*p == charToFind)
		{
			*pdwPos = dwLen-1;
			return TRUE;
		}
		
		--p;
		--dwLen;
	}
	
	return FALSE;
}

void FT_WordWrapReverseNLinesA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwMaxWidth, DWORD *pdwLinesToScroll, DWORD *pdwScrollPos)
{
	DWORD dwStart, dwStartOfLine, dwEndOfPrevLine, dwLineStart,
		dwLineCount, dwLinesToScroll;
		
	dwLineCount = 0;
	dwLinesToScroll = *pdwLinesToScroll;
	dwStartOfLine = *pdwScrollPos;
	dwEndOfPrevLine = dwStartOfLine-1;
	
	if (dwEndOfPrevLine && pcszStr[dwEndOfPrevLine] == LINEFEED_CHAR)
		--dwEndOfPrevLine;
	
	// Find the first line that begins after after a linefeed and is at 
	// or before the line we wish to scroll to.
	do {
	
		// Search backward for a LINEFEED_CHAR.  If we find one, our staring
		// postion is the character after the LINEFEED_CHAR, otherwise out 
		// starting position is the start of the field's text.
		if (_FT_FindReverseCharA(pcszStr, LINEFEED_CHAR, dwEndOfPrevLine, &dwStart))
			++dwStart;	// skip over the LINEFEED_CHAR
		else
			dwStart = 0;	// start of text string
			
		// Count the number of lines we move back through.
		dwLineStart = dwStart;
		while (dwLineStart < dwStartOfLine)
		{
			++dwLineCount;
			dwLineStart += FT_WordWrapA(ftFace, dwFontStyleFlags,
				&pcszStr[dwLineStart], MGLSTRLEN_INFINITE, dwMaxWidth);
		}

		if (dwStart <= 1)
		{
			// Is the first line a linefeed?
			if (dwStart == 1)
			{
				dwStart = 0;
				++dwLineCount;
			}
			
			break;
		}
		
		dwStartOfLine = dwStart;
		dwEndOfPrevLine = dwStartOfLine-2;	// move before linefeed
		
	} while (dwLineCount < dwLinesToScroll);
	
	// If we're moved back to many line, move foreward until we're at the 
	// correct line.
	while (dwLineCount < dwLinesToScroll)
	{
		dwStart += FT_WordWrapA(ftFace, dwFontStyleFlags,
			&pcszStr[dwStart], MGLSTRLEN_INFINITE, dwMaxWidth);
		--dwLineCount;
	}
	
	// Return results
	if (dwLineCount < dwLinesToScroll)
		*pdwLinesToScroll = dwLineCount;
		
	*pdwScrollPos = dwStart;
}

void FT_WordWrapReverseNLinesW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwMaxWidth, DWORD *pdwLinesToScroll, DWORD *pdwScrollPos)
{
	DWORD dwStart, dwStartOfLine, dwEndOfPrevLine, dwLineStart,
		dwLineCount, dwLinesToScroll;
		
	dwLineCount = 0;
	dwLinesToScroll = *pdwLinesToScroll;
	dwStartOfLine = *pdwScrollPos;
	dwEndOfPrevLine = dwStartOfLine-1;
	
	if (dwEndOfPrevLine && pcwszStr[dwEndOfPrevLine] == LINEFEED_CHAR)
		--dwEndOfPrevLine;
	
	// Find the first line that begins after after a linefeed and is at 
	// or before the line we wish to scroll to.
	do {
	
		// Search backward for a LINEFEED_CHAR.  If we find one, our staring
		// postion is the character after the LINEFEED_CHAR, otherwise out 
		// starting position is the start of the field's text.
		if (_FT_FindReverseCharW(pcwszStr, LINEFEED_CHAR, dwEndOfPrevLine, &dwStart))
			++dwStart;	// skip over the LINEFEED_CHAR
		else
			dwStart = 0;	// start of text string
			
		// Count the number of lines we move back through.
		dwLineStart = dwStart;
		while (dwLineStart < dwStartOfLine)
		{
			++dwLineCount;
			dwLineStart += FT_WordWrapW(ftFace, dwFontStyleFlags,
				&pcwszStr[dwLineStart], MGLSTRLEN_INFINITE, dwMaxWidth);
		}

		if (dwStart <= 1)
		{
			// Is the first line a linefeed?
			if (dwStart == 1)
			{
				dwStart = 0;
				++dwLineCount;
			}
			
			break;
		}
		
		dwStartOfLine = dwStart;
		dwEndOfPrevLine = dwStartOfLine-2;	// move before linefeed
		
	} while (dwLineCount < dwLinesToScroll);
	
	// If we're moved back to many line, move foreward until we're at the 
	// correct line.
	while (dwLineCount < dwLinesToScroll)
	{
		dwStart += FT_WordWrapW(ftFace, dwFontStyleFlags,
			&pcwszStr[dwStart], MGLSTRLEN_INFINITE, dwMaxWidth);
		--dwLineCount;
	}
	
	// Return results
	if (dwLineCount < dwLinesToScroll)
		*pdwLinesToScroll = dwLineCount;
		
	*pdwScrollPos = dwStart;
}

MGLTEXTLINENODE* FT_FormatTextA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth)
{
	MGLTEXTLINENODE *pHead, *pNode, *pLastNode;
	DWORD dwBreakOffset, dwLastOffset, dwLineLength;
	
	pHead = NULL;
	dwBreakOffset = 0;
	dwLastOffset = 0;
	
	// Keep formatting lines until we reach the end of the text.
	while (dwBreakOffset < dwStrLen)
	{
		dwLineLength = FT_WordWrapA(ftFace, dwFontStyleFlags, &pcszStr[dwLastOffset],
			dwStrLen-dwLastOffset, dwMaxWidth);
			
		// We must have hit the end of the string early (this is the case
		// when dwStrLen == MGLSTRLEN_INFINITE) so stop wrapping.
		if (dwLineLength == 0) break;
		
		// Adjust the break offset, adding the length of the wordwrapped line.
		dwBreakOffset += dwLineLength;
		
		// -----------------------------------------------------
		// Create a new node to store the word wrapping info.
		// -----------------------------------------------------
		pNode = (MGLTEXTLINENODE*)ftsl_malloc(sizeof(MGLTEXTLINENODE));
		if (!pNode) goto error;	// bail out
		
		// -----------------------------------
		// Init the node
		// -----------------------------------
		pNode->dwStartOffset = dwLastOffset;
		pNode->dwLength = dwLineLength;
		pNode->pNext = NULL;
		
		// -----------------------------------
		// Attach the node in place
		// -----------------------------------
		if (!pHead)
			pHead = pNode;
		else
		{
			// Traverse the list to find the last node
			pLastNode = pHead;
			
			while (pLastNode->pNext)
				pLastNode = pLastNode->pNext;
				
			// Attach the new node to the last node
			pLastNode->pNext = pNode;
		}
		
		// Update the last wordwrapping offset
		dwLastOffset = dwBreakOffset;
	}
	
	// If there's no wordwapping info, then make a linked list anyway.
	if (!pHead)
	{
		// -----------------------------------------------------
		// Create a new node to store the word wrapping info.
		// -----------------------------------------------------
		pNode = (MGLTEXTLINENODE*)ftsl_malloc(sizeof(MGLTEXTLINENODE));
		if (!pNode) goto error;	// bail out
		
		// -----------------------------------
		// Init the node
		// -----------------------------------
		pNode->dwStartOffset = 0;
		pNode->dwLength = 0;
		pNode->pNext = NULL;
		
		// -----------------------------------
		// Attach the node in place
		// -----------------------------------
		pHead = pNode;
	}
	
	return pHead;

	error:
	
	// Free all nodes
	FT_DoneFormatText(pHead);
	
	return NULL;
}

MGLTEXTLINENODE* FT_FormatTextW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth)
{
	MGLTEXTLINENODE *pHead, *pNode, *pLastNode;
	DWORD dwBreakOffset, dwLastOffset, dwLineLength;
	
	pHead = NULL;
	dwBreakOffset = 0;
	dwLastOffset = 0;
	
	// Keep formatting lines until we reach the end of the text.
	while (dwBreakOffset < dwStrLen)
	{
		dwLineLength = FT_WordWrapW(ftFace, dwFontStyleFlags, &pcwszStr[dwLastOffset],
			dwStrLen-dwLastOffset, dwMaxWidth);
			
		// We must have hit the end of the string early (this is the case
		// when dwStrLen == MGLSTRLEN_INFINITE) so stop wrapping.
		if (dwLineLength == 0) break;
		
		// Adjust the break offset, adding the length of the wordwrapped line.
		dwBreakOffset += dwLineLength;
		
		// -----------------------------------------------------
		// Create a new node to store the word wrapping info.
		// -----------------------------------------------------
		pNode = (MGLTEXTLINENODE*)ftsl_malloc(sizeof(MGLTEXTLINENODE));
		if (!pNode) goto error;	// bail out
		
		// -----------------------------------
		// Init the node
		// -----------------------------------
		pNode->dwStartOffset = dwLastOffset;
		pNode->dwLength = dwLineLength;
		pNode->pNext = NULL;
		
		// -----------------------------------
		// Attach the node in place
		// -----------------------------------
		if (!pHead)
			pHead = pNode;
		else
		{
			// Traverse the list to find the last node
			pLastNode = pHead;
			
			while (pLastNode->pNext)
				pLastNode = pLastNode->pNext;
				
			// Attach the new node to the last node
			pLastNode->pNext = pNode;
		}
		
		// Update the last wordwrapping offset
		dwLastOffset = dwBreakOffset;
	}
	
	// If there's no wordwapping info, then make a linked list anyway.
	if (!pHead)
	{
		// -----------------------------------------------------
		// Create a new node to store the word wrapping info.
		// -----------------------------------------------------
		pNode = (MGLTEXTLINENODE*)ftsl_malloc(sizeof(MGLTEXTLINENODE));
		if (!pNode) goto error;	// bail out
		
		// -----------------------------------
		// Init the node
		// -----------------------------------
		pNode->dwStartOffset = 0;
		pNode->dwLength = 0;
		pNode->pNext = NULL;
		
		// -----------------------------------
		// Attach the node in place
		// -----------------------------------
		pHead = pNode;
	}
	
	return pHead;

	error:
	
	// Free all nodes
	FT_DoneFormatText(pHead);
	
	return NULL;
}

void FT_DoneFormatText(MGLTEXTLINENODE *pHead)
{
	MGLTEXTLINENODE *pNode, *pNext;
	
	pNode = pHead;
	while (pNode)
	{
		// Store the next ptr
		pNext = pNode->pNext;
	
		// Free the current node
		ftsl_free(pNode);
	
		// Advance to the next node
		pNode = pNext;
	}
}

void FT_WidthToOffsetA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD dwWidth,
	BOOL *pbLeadingEdge /*OUT*/, DWORD *pdwTruncWidth /*OUT*/, DWORD *pdwOffset /*OUT*/)
{
	CHAR *p;
	WCHAR rgwStr[2];
	LONG srclen;
	DWORD dwCharWidth;
	DWORD dwCurrWidth, dwOffset;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (CHAR*)pcszStr;
	dwCurrWidth = 0;
	dwOffset = 0;
	
	// Compute length of the string. This helps us keep the code
	// cleaner, especially so we don't have to have special handling
	// for the case when (dwStrLen == MGLSTRLEN_INFINITE).
	if (dwStrLen == MGLSTRLEN_INFINITE)
	{
		DWORD dwStrLenActual = S_StrLenA(pcszStr);
		if (dwStrLenActual < dwStrLen) dwStrLen = dwStrLenActual;
	}
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (dwOffset < dwStrLen)
	{	
		dwCharWidth = 0;
		
		// --------------------------------------
		
		// Determine the length of the character that
		// we need to convert to Unicode.
		srclen = (S_IsDBCSLeadByte(*p) && *(p+1)) ? 2 : 1;
	
		// Convert the current character to Unicode.
		// If an error occurred, just skip to the next character.
		if (S_MultiByteToWideChar(CP_ACP, 0, p, srclen, rgwStr, sizeof(rgwStr)) == 0)
			goto nextChar;
		
		// ---------------------------------------------------
		// Retrieve width of the current character
		// ----------------------------------------------------
		if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*rgwStr,
			ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
			&dwCharWidth, &ftPreviousGlyphIndex))
		{
			// An error occurred. Just advance to the next character.
			goto nextChar;
		}
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		
		dwCurrWidth += dwCharWidth;
		
		if (dwCurrWidth <= dwWidth) {
			dwOffset += srclen;
			p += srclen;
		} else {
			break;
		}
	}
	
	// If we ran over the end of the string, we know that the leading
	// edge result must be false, and we also want to prune back the
	// returned offset to be the length (just in case we got passed a
	// bogus double-byte character at the end of the string).
	if (dwOffset >= dwStrLen)
	{
		if (pbLeadingEdge)
			*pbLeadingEdge = TRUE;
		
		if (pdwTruncWidth)
			*pdwTruncWidth = dwCurrWidth;
		
		*pdwOffset = dwStrLen;
	}
	else
	{
		if (pbLeadingEdge)
			*pbLeadingEdge = (dwCurrWidth - dwWidth > (dwCharWidth / 2));
	
		if (pdwTruncWidth)
			*pdwTruncWidth = dwCurrWidth - dwCharWidth;
		
		*pdwOffset = dwOffset;
	}
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
}

void FT_WidthToOffsetW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwWidth,
	BOOL *pbLeadingEdge /*OUT*/, DWORD *pdwTruncWidth /*OUT*/, DWORD *pdwOffset /*OUT*/)
{
	WCHAR *p;
	DWORD dwCharWidth;
	DWORD dwCurrWidth, dwOffset;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (WCHAR*)pcwszStr;
	dwCurrWidth = 0;
	dwOffset = 0;
	
	// Compute length of the string. This helps us keep the code
	// cleaner, especially so we don't have to have special handling
	// for the case when (dwStrLen == MGLSTRLEN_INFINITE).
	if (dwStrLen == MGLSTRLEN_INFINITE)
	{
		DWORD dwStrLenActual = S_StrLenW(pcwszStr);
		if (dwStrLenActual < dwStrLen) dwStrLen = dwStrLenActual;
	}
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (dwOffset < dwStrLen)
	{	
		dwCharWidth = 0;
		
		// ---------------------------------------------------
		// Retrieve width of the current character
		// ----------------------------------------------------
		if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*p,
			ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
			&dwCharWidth, &ftPreviousGlyphIndex))
		{
			// An error occurred. Just advance to the next character.
			goto nextChar;
		}
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		
		dwCurrWidth += dwCharWidth;
		
		if (dwCurrWidth <= dwWidth) {
			++dwOffset;
			++p;
		} else {
			break;
		}
	}
	
	// If we ran over the end of the string, we know that the leading
	// edge result must be false, and we also want to prune back the
	// returned offset to be the length (just in case we got passed a
	// bogus double-byte character at the end of the string).
	if (dwOffset >= dwStrLen)
	{
		if (pbLeadingEdge)
			*pbLeadingEdge = TRUE;
		
		if (pdwTruncWidth)
			*pdwTruncWidth = dwCurrWidth;
		
		*pdwOffset = dwStrLen;
	}
	else
	{
		if (pbLeadingEdge)
			*pbLeadingEdge = (dwCurrWidth - dwWidth > (dwCharWidth / 2));
	
		if (pdwTruncWidth)
			*pdwTruncWidth = dwCurrWidth - dwCharWidth;
		
		*pdwOffset = dwOffset;
	}
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
}

void FT_CharsInWidthA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD *pdwStrWidth /*IN & OUT*/,
	DWORD *pdwStrLen /*IN & OUT*/, BOOL *pbFitWithinWidth /*OUT*/)
{
	CHAR *p;
	WCHAR rgwStr[2];
	LONG srclen;
	DWORD dwCharWidth;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	DWORD dwStrLenActual;
	DWORD dwCurrWidth, dwLength, dwMaxLength, dwMaxWidth;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (CHAR*)pcszStr;
	
	// Compute length of the string. This helps us keep the code
	// cleaner, especially so we don't have to have special handling
	// for the case when (dwStrLen == MGLSTRLEN_INFINITE).
	dwStrLenActual = S_StrLenA(pcszStr);
	
	dwCurrWidth = 0;
	dwLength = 0;
	dwMaxLength = (dwStrLenActual < *pdwStrLen) ? dwStrLenActual : *pdwStrLen;
	dwMaxWidth = *pdwStrWidth;
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (*p && *p != LINEFEED_CHAR &&
		dwLength < dwMaxLength)
	{
		dwCharWidth = 0;
		
		// --------------------------------------
		
		// Determine the length of the character that
		// we need to convert to Unicode.
		srclen = (S_IsDBCSLeadByte(*p) && *(p+1)) ? 2 : 1;
	
		// Convert the current character to Unicode.
		// If an error occurred, just skip to the next character.
		if (S_MultiByteToWideChar(CP_ACP, 0, p, srclen, rgwStr, sizeof(rgwStr)) == 0)
			goto nextChar;
			
		// ---------------------------------------------------
		// Retrieve width of the current character
		// ----------------------------------------------------
		if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*rgwStr,
			ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
			&dwCharWidth, &ftPreviousGlyphIndex))
		{
			// An error occurred. Just advance to the next character.
			goto nextChar;
		}
			
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		
		if (dwCurrWidth + dwCharWidth <= dwMaxWidth)
			{
			dwCurrWidth += dwCharWidth;
			dwLength += srclen /*size of multibyte char*/;
			}
		else
			break;			// can't add any more characters.
		
		p += srclen /*size of multibyte char*/;
	}
	
	// string is the first character that didn't get included.  A '\0'
	// means the entire string fit, a linefeedChr means the string wrapped
	// (didn't fit), and anything else means the string was too wide.
	// Now, if the string is too wide because of blank characters, then
	// the string shouldn't considered to be truncated.  It's entire visible
	// contents does fit.
	
	// If the character we were adding was a whitespace and
	// all the characters until the end of the string are white spaces
	// then we don't consider the string truncated.
	if (dwLength >= dwMaxLength)		// used up all the characters
		*pbFitWithinWidth = TRUE;
	else if (*p == LINEFEED_CHAR)
		*pbFitWithinWidth = FALSE;
	else if (*p == '\0')
		*pbFitWithinWidth = TRUE;
	else
	{
		CHAR *s2 = p;
		
		while (*s2 == ' ' || *s2 == HORIZ_TAB_CHAR)
			s2++;
		
		if (*s2 == '\0')
			*pbFitWithinWidth = TRUE;
		else
			*pbFitWithinWidth = FALSE;
	}
	
	// Visual optimization.  If the last char was unseen remove it.
	// Drawing it adds nothing of value.
	--p;
	while (dwLength > 0 && 
		(*p == SPACE_CHAR || *p == LINEFEED_CHAR || *p == HORIZ_TAB_CHAR))
	{
		CHAR *previous;
	
		// NOTE: We don't need to convert to Unicode here
		// since we're only checking for spaces, linefeeds, and tabs.
		
		// Get the previous (preceding) character. We don't know if the previous char
		// is the end of a multibyte char, so use CharPrevA().
		previous = S_CharPrevA(pcszStr, p);
		
		if (p == previous)
		{
			// We're already at the beginning of the string,
			// so there's no kerning to handle.
			ftPreviousGlyphIndex = 0;
		}
		else
			ftPreviousGlyphIndex = FT_Get_Char_Index(ftf, (FT_ULong)*previous);
		
		// ---------------------------------------------------
		// Retrieve width of the current character
		// ----------------------------------------------------
		if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*p,
			ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
			&dwCharWidth, NULL))
		{
			// An error occurred. In this *RARE* case (considering that we know
			// which characters we're processing), the character width is simply 0.
			dwCharWidth = 0;
		}
	
		dwCurrWidth -= dwCharWidth;
		
		--p;
		--dwLength;
	}
	
	// Return values
	*pdwStrWidth = dwCurrWidth;
	*pdwStrLen = dwLength;
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
}

void FT_CharsInWidthW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD *pdwStrWidth /*IN & OUT*/,
	DWORD *pdwStrLen /*IN & OUT*/, BOOL *pbFitWithinWidth /*OUT*/)
{
	WCHAR *p;
	DWORD dwCharWidth;
	FT_Face ftf;
	FT_Int32 ftLoadGlyphFlags;
	FT_UInt ftPreviousGlyphIndex;
	FT_Bool ftUseKerning;
	DWORD dwStrLenActual;
	DWORD dwCurrWidth, dwLength, dwMaxLength, dwMaxWidth;
	
	ftf = (FT_Face)ftFace;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_MONOCHROME;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (WCHAR*)pcwszStr;
	
	// Compute length of the string. This helps us keep the code
	// cleaner, especially so we don't have to have special handling
	// for the case when (dwStrLen == MGLSTRLEN_INFINITE).
	dwStrLenActual = S_StrLenW(pcwszStr);
	
	dwCurrWidth = 0;
	dwLength = 0;
	dwMaxLength = (dwStrLenActual < *pdwStrLen) ? dwStrLenActual : *pdwStrLen;
	dwMaxWidth = *pdwStrWidth;
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (*p && *p != LINEFEED_CHAR &&
		dwLength < dwMaxLength)
	{
		dwCharWidth = 0;
			
		// ---------------------------------------------------
		// Retrieve width of the current character
		// ----------------------------------------------------
		if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*p,
			ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
			&dwCharWidth, &ftPreviousGlyphIndex))
		{
			// An error occurred. Just advance to the next character.
			goto nextChar;
		}
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		
		if (dwCurrWidth + dwCharWidth <= dwMaxWidth)
			{
			dwCurrWidth += dwCharWidth;
			++dwLength;
			}
		else
			break;			// can't add any more characters.
		
		p = S_CharNextW(p);
	}
	
	// string is the first character that didn't get included.  A '\0'
	// means the entire string fit, a linefeedChr means the string wrapped
	// (didn't fit), and anything else means the string was too wide.
	// Now, if the string is too wide because of blank characters, then
	// the string shouldn't considered to be truncated.  It's entire visible
	// contents does fit.
	
	// If the character we were adding was a whitespace and
	// all the characters until the end of the string are white spaces
	// then we don't consider the string truncated.
	if (dwLength >= dwMaxLength)		// used up all the characters
		*pbFitWithinWidth = TRUE;
	else if (*p == LINEFEED_CHAR)
		*pbFitWithinWidth = FALSE;
	else if (*p == '\0')
		*pbFitWithinWidth = TRUE;
	else
	{
		WCHAR *s2 = p;
		
		while (*s2 == ' ' || *s2 == HORIZ_TAB_CHAR)
			s2++;
		
		if (*s2 == '\0')
			*pbFitWithinWidth = TRUE;
		else
			*pbFitWithinWidth = FALSE;
	}
	
	// Visual optimization.  If the last char was unseen remove it.
	// Drawing it adds nothing of value.
	--p;
	while (dwLength > 0 && 
		(*p == SPACE_CHAR || *p == LINEFEED_CHAR || *p == HORIZ_TAB_CHAR))
	{
		WCHAR *previous;
	
		// NOTE: We don't need to convert to Unicode here
		// since we're only checking for spaces, linefeeds, and tabs.
		
		// Get the previous (preceding) character. We don't know if the previous char
		// is the end of a multibyte char, so use CharPrevA().
		previous = S_CharPrevW(pcwszStr, p);
		
		if (p == previous)
		{
			// We're already at the beginning of the string,
			// so there's no kerning to handle.
			ftPreviousGlyphIndex = 0;
		}
		else
			ftPreviousGlyphIndex = FT_Get_Char_Index(ftf, (FT_ULong)*previous);
		
		// ---------------------------------------------------
		// Retrieve width of the current character
		// ----------------------------------------------------
		if (!_FT_GetCharWidth(ftf, dwFontStyleFlags, (FT_ULong)*p,
			ftLoadGlyphFlags, ftUseKerning, ftPreviousGlyphIndex,
			&dwCharWidth, NULL))
		{
			// An error occurred. In this *RARE* case (considering that we know
			// which characters we're processing), the character width is simply 0.
			dwCharWidth = 0;
		}
	
		dwCurrWidth -= dwCharWidth;
		
		--p;
		--dwLength;
	}
	
	// Return values
	*pdwStrWidth = dwCurrWidth;
	*pdwStrLen = dwLength;
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
}

// Delete a FreeType font face.
// Returns: FreeType error code.
FTERR FT_DeleteFontFace(FTFACE ftFace, FTGLYPHCACHE ftGlyphCache)
{
	FT_Error ftError;
	
	// Verify parameters
	if (!ftFace) return FT_Err_Invalid_Argument;
	
	// Delete FreeType font face
	ftError = FT_Done_Face ((FT_Face)ftFace);
	if (ftError != FT_Err_Ok) return ftError;
	
	// Delete glyph cache, if it exists
	if (ftGlyphCache)
	{
		// NOTE: This function doesn't really fail.
		FT_DeleteGlyphCache((FT_CacheHandle)ftGlyphCache);
	}

	return FT_Err_Ok;
}

#ifdef _PRGM_SECT
#pragma mark ------------ Text Drawing Functions --------------
#endif

// Initializes the specified GLYPHDATA structure based on data
// stored within a GLYPHCACHEDATA structutre.
inline
void _FT_InitGlyphDataFromCache(GLYPHDATA *pGlyphData, const GLYPHCACHEDATA *pGlyphCacheData)
{
	pGlyphData->dwWidth = pGlyphCacheData->dwWidth;
	pGlyphData->dwHeight = pGlyphCacheData->dwHeight;
	pGlyphData->nLeft = pGlyphCacheData->nLeft;
	pGlyphData->nTop = pGlyphCacheData->nTop;
	
	pGlyphData->xAdvance = pGlyphCacheData->xAdvance;
	pGlyphData->yAdvance = pGlyphCacheData->yAdvance;
	
	pGlyphData->pixelMode = pGlyphCacheData->pixelMode;
	pGlyphData->maxGrays = pGlyphCacheData->maxGrays;
	
	pGlyphData->pitch = pGlyphCacheData->pitch;
	pGlyphData->pBuffer = pGlyphCacheData->pBuffer;
}

// This function first attempts to get the character glyph bitmap from the glyph cache.
// If that fails, this function creates a character glyph bitmap. If a cache exists, this function
// adds the newly created glyph bitmap for faster access next time. The returned
inline
FT_Error _FT_GetCharGlyph(FT_Library ftlib, FT_CacheHandle ftCache, FT_UInt ftGlyphIndex,
	FT_Face ftf, DWORD dwFontStyleFlags, GLYPHDATA *pGlyphData /*OUT*/)
{
	FT_GlyphSlot ftSlot;
	FT_Error ftError;
	GLYPHCACHEDATA glyphCacheData;
	
	// If a cache exists, attempt to get the glyph from it.
	if (ftCache)
	{
		ftError = FT_LookupGlyphCacheEntry(ftCache, ftGlyphIndex, &glyphCacheData);
		if (ftError == FT_Err_Ok)
		{
			// Init the GLYPHDATA from the cache
			_FT_InitGlyphDataFromCache(pGlyphData, &glyphCacheData);
			
			// All done!
			return FT_Err_Ok;
		}
	}
	
	ftSlot = ftf->glyph; /* a small shortcut */
	
	// Render the glyph
	ftError = FT_Render_Glyph( ftf->glyph, (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS) ?
		FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO );
	if (ftError != FT_Err_Ok) return ftError;	// bail out
	
	if (ftSlot->bitmap.buffer)
	{
		// Adjust for BOLD typeface. (No need to adjust
		// for ITALIC typeface since it's just a transformation.)
		if (dwFontStyleFlags & MGLFONTSTYLE_BOLD)
		{
			FT_Pos ftXStrength, ftYStrength;
			FT_Pos ftXAdjust;
			
			FTHelper::ComputeBoldStrength(ftSlot->advance.x, ftSlot->advance.y, &ftXStrength, &ftYStrength);
			
			// Make the bitmap bold
			FT_Bitmap_Embolden(ftlib, &ftSlot->bitmap, ftXStrength, ftYStrength );
			
			// Adjust the X draw position to account for the bold character spacing
			FTHelper::ComputeBoldAdjust(ftSlot->advance.x, &ftXAdjust);
			
			// Adjust glyph width
			ftSlot->advance.x += ftXAdjust;
		}
	}
	
	// ----------------------------------------------------------------
	// Add the glyph to the cache, if the cache exists
	// ----------------------------------------------------------------
	if (ftCache)
	{
		ftError = FT_AddGlyphCacheEntry(ftCache, ftGlyphIndex, ftSlot);
		if (ftError != FT_Err_Ok) return ftError;	// bail out
	}
	
	// ------------------------
	// Return glyph data
	// -----------------------
	if (ftCache)
	{
		// Grab the glyph data from the cache. Yes, this seems kind
		// of redundant, but we need to do it for FT_ReleaseGlyphCacheEntry().
		// There shouldn't be much of a performance issue with this since the glyph
		// is at the beginning of the list anyway. - ptm
		ftError = FT_LookupGlyphCacheEntry(ftCache, ftGlyphIndex, &glyphCacheData);
		if (ftError != FT_Err_Ok) return ftError;	// bail out
		
		// Init the GLYPHDATA from the cache
		_FT_InitGlyphDataFromCache(pGlyphData, &glyphCacheData);
	}
	else
	{
		// There's no cache, so just grab the data directly
		// from the GlyphSlot.
		
		pGlyphData->dwWidth = ftSlot->bitmap.width;
		pGlyphData->dwHeight = ftSlot->bitmap.rows;
		pGlyphData->nLeft = ftSlot->bitmap_left;
		pGlyphData->nTop = ftSlot->bitmap_top;
		
		pGlyphData->xAdvance = ftSlot->advance.x;
		pGlyphData->yAdvance = ftSlot->advance.y;
		
		pGlyphData->pixelMode = ftSlot->bitmap.pixel_mode;
		pGlyphData->maxGrays = ftSlot->bitmap.num_grays;
		
		pGlyphData->pitch = ftSlot->bitmap.pitch;
		pGlyphData->pBuffer = ftSlot->bitmap.buffer;
	}
	
	return FT_Err_Ok;
}

inline
void _FT_DrawCharGlyph(DWORD dwFontStyleFlags, LONG nAscent, GLYPHDATA *pGlyphData /*OUT*/,
	void *dest, LONG destXPitch, LONG destYPitch, LONG &destX, LONG &destY,
	LONG destWidth, LONG destHeight, RECT *pClipRect, DWORD dwNativeColor,
	DWORD dwFlags, DWORD dwOpacity, DWORD dwNativeShadowColor, DWORD dwShadowOpacity)
{
	if (pGlyphData->pBuffer)
	{
		LONG dX, dY;
		BOOL bUseOpacity;
		
		bUseOpacity = (dwFlags & MGLDRAWTEXT_OPACITY);
		
		// Compute current blit positions for the glyph
		dX = destX + pGlyphData->nLeft;
		dY = (destY - pGlyphData->nTop) + nAscent;
		
		// Draw the "shadow" if needed
		if (dwFlags & MGLDRAWTEXT_SHADOW)
		{
			if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
			{
				DrawText565::DrawFTBmp8(dest, destXPitch, destYPitch, destWidth, destHeight, dX+1, dY+1,
					pGlyphData->pBuffer, pGlyphData->pitch, pGlyphData->dwWidth, pGlyphData->dwHeight,
					pClipRect, dwNativeShadowColor, bUseOpacity, dwShadowOpacity);
			}
			else
			{
				DrawText565::DrawFTBmp1(dest, destXPitch, destYPitch, destWidth, destHeight, dX+1, dY+1,
					pGlyphData->pBuffer, pGlyphData->pitch, pGlyphData->dwWidth, pGlyphData->dwHeight,
					pClipRect, dwNativeShadowColor, bUseOpacity, dwShadowOpacity);
			}
		}
		
		// Blit the original character glyph
		if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		{
			DrawText565::DrawFTBmp8(dest, destXPitch, destYPitch, destWidth, destHeight, dX, dY,
				pGlyphData->pBuffer, pGlyphData->pitch, pGlyphData->dwWidth, pGlyphData->dwHeight,
				pClipRect, dwNativeColor, bUseOpacity, dwOpacity);
		}
		else
		{
			DrawText565::DrawFTBmp1(dest, destXPitch, destYPitch, destWidth, destHeight, dX, dY,
				pGlyphData->pBuffer, pGlyphData->pitch, pGlyphData->dwWidth, pGlyphData->dwHeight,
				pClipRect, dwNativeColor, bUseOpacity, dwOpacity);
		}
	}
	
	// Adjust draw position
	destX += pGlyphData->xAdvance >> 6;
	destY += pGlyphData->yAdvance >> 6; /* not useful for now */
}

inline
FT_Error _FT_ReleaseCharGlyph(FT_CacheHandle ftCache, FT_UInt ftGlyphIndex, FT_Face ftf)
{
	FT_Error ftError;

	if (ftCache)
	{
		// Release the character glyph in the cache.
		ftError = FT_ReleaseGlyphCacheEntry(ftCache, ftGlyphIndex);
		if (ftError != FT_Err_Ok) return ftError;	// bail out
	}
	else
	{
		// No need to free the previously created glyph bitmap.
		// FreeType does it for us!
	}
		
	return FT_Err_Ok;
}

void FT_DrawTextA_565(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, LONG destWidth, LONG destHeight, RECT *pClipRect, DWORD dwNativeColor,
	DWORD dwFlags, DWORD dwOpacity, DWORD dwNativeShadowColor, DWORD dwShadowOpacity)
{
	CHAR *p;
	WCHAR rgwStr[2];
	LONG srclen;
	LONG nAscent;
	FT_Library ftlib;
	FT_Face ftf;
	FT_CacheHandle ftc;
	FT_Int32 ftLoadGlyphFlags;
	FT_Render_Mode ftRenderMode;
	FT_UInt ftGlyphIndex, ftPreviousGlyphIndex;
	FT_Error ftError;
	FT_Bool ftUseKerning;
	GLYPHDATA glyphData;
	
	ftlib = (FT_Library)ftLibrary;
	ftf = (FT_Face)ftFace;
	ftc = (FT_CacheHandle)ftCache;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_TARGET_MONO;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
	
	// ---------------------------------------------------------------------
	// Build the FT render mode from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftRenderMode = FT_RENDER_MODE_NORMAL;
	else
		ftRenderMode = FT_RENDER_MODE_MONO;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (CHAR*)pcszStr;
	nAscent = ftf->size->metrics.ascender >> 6;
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (*p && dwStrLen)
	{
		// OPTIMIZATION: Are we done drawing?
		if (destX >= pClipRect->right) break;
		
		// Determine the length of the character that
		// we need to convert to Unicode.
		srclen = (S_IsDBCSLeadByte(*p) && *(p+1)) ? 2 : 1;
	
		// Convert the current character to Unicode.
		// If an error occurred, just skip to the next character.
		if (S_MultiByteToWideChar(CP_ACP, 0, p, srclen, rgwStr, sizeof(rgwStr)) == 0)
			goto nextChar;
		
		// Retrieve glyph index from character code
		ftGlyphIndex = FT_Get_Char_Index(ftf, (FT_ULong)*rgwStr);
		
		// -----------------------------------------
		// Load the glyph
		// -----------------------------------------
		ftError = FT_Load_Glyph(ftf, ftGlyphIndex, ftLoadGlyphFlags);
		if (ftError != FT_Err_Ok) goto nextChar;	// ignore errors
		
		// Retrieve kerning distance and move pen position
		if (ftUseKerning && ftPreviousGlyphIndex && ftGlyphIndex)
		{
			FT_Vector ftDelta;
			
			FT_Get_Kerning(ftf, ftPreviousGlyphIndex, ftGlyphIndex, FT_KERNING_DEFAULT, &ftDelta );
			destX += ftDelta.x >> 6;
		}
		
		// -----------------------------------------
		// Get/render the glyph
		// -----------------------------------------
		ftError = _FT_GetCharGlyph(ftlib, ftc, ftGlyphIndex, ftf, dwFontStyleFlags, &glyphData);
		if (ftError != FT_Err_Ok) goto nextChar;	// ignore errors
		
		// -----------------------------------------
		// Blit the glyph
		// -----------------------------------------
		_FT_DrawCharGlyph(dwFontStyleFlags, nAscent, &glyphData, dest, destXPitch, destYPitch, destX, destY,
			destWidth, destHeight, pClipRect, dwNativeColor, dwFlags, dwOpacity, dwNativeShadowColor, dwShadowOpacity);
		
		// -----------------------------------------
		// Release the glyph
		// -----------------------------------------
		_FT_ReleaseCharGlyph(ftc, ftGlyphIndex, ftf);
		
		// Store the current glyph index as the previous, used for kerning
		ftPreviousGlyphIndex = ftGlyphIndex;
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		p += srclen; /*S_CharNextA(p);*/
		dwStrLen -= srclen;
	}
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
}

void FT_DrawTextW_565(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, LONG destWidth, LONG destHeight, RECT *pClipRect, DWORD dwNativeColor,
	DWORD dwFlags, DWORD dwOpacity, DWORD dwNativeShadowColor, DWORD dwShadowOpacity)
{
	WCHAR *p;
	LONG nAscent;
	FT_Library ftlib;
	FT_Face ftf;
	FT_CacheHandle ftc;
	FT_Int32 ftLoadGlyphFlags;
	FT_Render_Mode ftRenderMode;
	FT_UInt ftGlyphIndex, ftPreviousGlyphIndex;
	FT_Error ftError;
	FT_Bool ftUseKerning;
	GLYPHDATA glyphData;
	
	ftlib = (FT_Library)ftLibrary;
	ftf = (FT_Face)ftFace;
	ftc = (FT_CacheHandle)ftCache;
	
	// ---------------------------------------------------------------------
	// Build the FT_Load_Glyph flags from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftLoadGlyphFlags = FT_LOAD_NO_BITMAP;
	else
		ftLoadGlyphFlags = FT_LOAD_TARGET_MONO;
		
	if (dwFontStyleFlags & MGLFONTSTYLE_NOHINTING)
		ftLoadGlyphFlags |= FT_LOAD_NO_HINTING;
	
	// ---------------------------------------------------------------------
	// Build the FT render mode from the font style
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ANTIALIAS)
		ftRenderMode = FT_RENDER_MODE_NORMAL;
	else
		ftRenderMode = FT_RENDER_MODE_MONO;
		
	// ---------------------------------------------------------------------
	// Initialize transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
	{
		FT_Matrix ftShear;
		
		// Compute ITALIC transformation matrix
		FTHelper::ComputeItalicMatrix(ftf, &ftShear);
		
		// Apply the transformation
		FT_Set_Transform(ftf, &ftShear, NULL);
	}

	p = (WCHAR*)pcwszStr;
	nAscent = ftf->size->metrics.ascender >> 6;
	
	ftUseKerning = FT_HAS_KERNING(ftf);
	ftPreviousGlyphIndex = 0;
	
	while (*p && dwStrLen)
	{
		// OPTIMIZATION: Are we done drawing?
		if (destX >= pClipRect->right) break;
	
		// Retrieve glyph index from character code
		ftGlyphIndex = FT_Get_Char_Index(ftf, (FT_ULong)*p);
		
		// -----------------------------------------
		// Load the glyph
		// -----------------------------------------
		ftError = FT_Load_Glyph(ftf, ftGlyphIndex, ftLoadGlyphFlags);
		if (ftError != FT_Err_Ok) goto nextChar;	// ignore errors
		
		// Retrieve kerning distance and move pen position
		if (ftUseKerning && ftPreviousGlyphIndex && ftGlyphIndex)
		{
			FT_Vector ftDelta;
			
			FT_Get_Kerning(ftf, ftPreviousGlyphIndex, ftGlyphIndex, FT_KERNING_DEFAULT, &ftDelta );
			destX += ftDelta.x >> 6;
		}
		
		// -----------------------------------------
		// Get/render the glyph
		// -----------------------------------------
		ftError = _FT_GetCharGlyph(ftlib, ftc, ftGlyphIndex, ftf, dwFontStyleFlags, &glyphData);
		if (ftError != FT_Err_Ok) goto nextChar;	// ignore errors
		
		// -----------------------------------------
		// Blit the glyph
		// -----------------------------------------
		_FT_DrawCharGlyph(dwFontStyleFlags, nAscent, &glyphData, dest, destXPitch, destYPitch, destX, destY,
			destWidth, destHeight, pClipRect, dwNativeColor, dwFlags, dwOpacity, dwNativeShadowColor, dwShadowOpacity);
		
		// -----------------------------------------
		// Release the glyph
		// -----------------------------------------
		_FT_ReleaseCharGlyph(ftc, ftGlyphIndex, ftf);
		
		// Store the current glyph index as the previous, used for kerning
		ftPreviousGlyphIndex = ftGlyphIndex;
		
		// -----------------------------------------
		// Advance to the next character
		// -----------------------------------------
		nextChar:
		p = S_CharNextW(p);
		--dwStrLen;
	}
	
	// ---------------------------------------------------------------------
	// Restore transformation matrix for ITALIC typeface
	// ---------------------------------------------------------------------
	if (dwFontStyleFlags & MGLFONTSTYLE_ITALIC)
		FT_Set_Transform(ftf, NULL, NULL);
}

// Helper function for pixel format independent (ANSI) text drawing
static
void _FT_DrawTextA(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	LONG destX, LONG destY, const CHAR *pcszStr, DWORD dwStrLen, MGLBUFFDESC *pDestBuffDesc, RECT *pClipRect,
	COLORREF dwColor, DWORD dwFlags, DWORD dwOpacity, COLORREF dwShadowColor, DWORD dwShadowOpacity)
{
	LONG destXPitch, destYPitch;
	DWORD dwNativeColor;
	DWORD dwNativeShadowColor;

	// Draw the text according to the pixel format
	switch (pDestBuffDesc->dwPixelFormat)
	{
		case MGLPIXELFORMAT_565:
		
			destXPitch = pDestBuffDesc->xPitch / sizeof(WORD);
			destYPitch = pDestBuffDesc->yPitch / sizeof(WORD);
			
			dwNativeColor = pxutil565::ColorrefToNative(dwColor);
			dwNativeShadowColor = pxutil565::ColorrefToNative(dwShadowColor);
			
			FT_DrawTextA_565(ftLibrary, ftFace, ftCache, dwFontStyleFlags, pcszStr, dwStrLen,
				pDestBuffDesc->pBuffer, destXPitch, destYPitch, destX, destY,
				pDestBuffDesc->dwWidth, pDestBuffDesc->dwHeight, pClipRect, dwNativeColor,
				dwFlags, dwOpacity, dwNativeShadowColor, dwShadowOpacity);
			
			break;
			
		default:
			break;
	}
}

// Helper function for pixel format independent (Unicode) text drawing
static
void _FT_DrawTextW(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	LONG destX, LONG destY, const WCHAR *pcwszStr, DWORD dwStrLen, MGLBUFFDESC *pDestBuffDesc, RECT *pClipRect,
	COLORREF dwColor, DWORD dwFlags, DWORD dwOpacity, COLORREF dwShadowColor, DWORD dwShadowOpacity)
{
	LONG destXPitch, destYPitch;
	DWORD dwNativeColor;
	DWORD dwNativeShadowColor;

	// Draw the text according to the pixel format
	switch (pDestBuffDesc->dwPixelFormat)
	{
		case MGLPIXELFORMAT_565:
		
			destXPitch = pDestBuffDesc->xPitch / sizeof(WORD);
			destYPitch = pDestBuffDesc->yPitch / sizeof(WORD);
			
			dwNativeColor = pxutil565::ColorrefToNative(dwColor);
			dwNativeShadowColor = pxutil565::ColorrefToNative(dwShadowColor);
			
			FT_DrawTextW_565(ftLibrary, ftFace, ftCache, dwFontStyleFlags, pcwszStr, dwStrLen,
				pDestBuffDesc->pBuffer, destXPitch, destYPitch, destX, destY,
				pDestBuffDesc->dwWidth, pDestBuffDesc->dwHeight, pClipRect, dwNativeColor,
				dwFlags, dwOpacity, dwNativeShadowColor, dwShadowOpacity);
			
			break;
			
		default:
			break;
	}
}

// Helper function for pixel format independent rectangle filling
static
void _FT_FillRect(MGLBUFFDESC *pDestBuffDesc, LONG x, LONG y, LONG rw, LONG rh, COLORREF dwColor)
{
	LONG destXPitch, destYPitch;
	DWORD dwNativeColor;

	// Fill the rectangle according to the pixel format
	switch (pDestBuffDesc->dwPixelFormat)
	{
		case MGLPIXELFORMAT_565:
		
			destXPitch = pDestBuffDesc->xPitch / sizeof(WORD);
			destYPitch = pDestBuffDesc->yPitch / sizeof(WORD);
		
			dwNativeColor = pxutil565::ColorrefToNative(dwColor);
		
			BL_FillRect565(pDestBuffDesc->pBuffer, destXPitch, destYPitch,
				pDestBuffDesc->dwWidth, pDestBuffDesc->dwHeight, x, y, rw, rh, dwNativeColor);
			
			break;
			
		default:
			break;
	}
}

// Helper function for pixel format independent rectangle filling (with opacity)
static
void _FT_FillRectOpacity(MGLBUFFDESC *pDestBuffDesc, LONG x, LONG y, LONG rw, LONG rh,
	COLORREF dwColor, DWORD dwOpacity)
{
	LONG destXPitch, destYPitch;
	DWORD dwNativeColor;

	// Fill the rectangle according to the pixel format
	switch (pDestBuffDesc->dwPixelFormat)
	{
		case MGLPIXELFORMAT_565:
		
			destXPitch = pDestBuffDesc->xPitch / sizeof(WORD);
			destYPitch = pDestBuffDesc->yPitch / sizeof(WORD);
		
			dwNativeColor = pxutil565::ColorrefToNative(dwColor);
		
			BL_FillRectOpacity565(pDestBuffDesc->pBuffer, destXPitch, destYPitch,
				pDestBuffDesc->dwWidth, pDestBuffDesc->dwHeight, x, y, rw, rh,
				dwNativeColor, dwOpacity);
			
			break;
			
		default:
			break;
	}
}

static
void _FT_DrawTextEllipsisA(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	LONG destX, LONG destY, const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth,
	MGLBUFFDESC *pDestBuffDesc, RECT *pClipRect, COLORREF dwColor, DWORD dwDrawTextFlags,
	DWORD dwOpacity, COLORREF dwShadowColor, DWORD dwShadowOpacity)
{
	const CHAR *szEllipsisStr = "...";
	DWORD dwEllipsisWidth;
	DWORD dwStrLenActual;
	DWORD dwTruncWidth, dwOffset;
	
	// Get width of ellipsis string
	FT_ComputeBBoxA(ftFace, dwFontStyleFlags, szEllipsisStr, MGLSTRLEN_INFINITE,
		&dwEllipsisWidth, NULL);
	
	// Compute length of the string. This helps us keep the code
	// cleaner, especially so we don't have to have special handling
	// for the case when (dwStrLen == MGLSTRLEN_INFINITE).
	if (dwStrLen == MGLSTRLEN_INFINITE)
	{
		dwStrLenActual = S_StrLenA(pcszStr);
		if (dwStrLenActual < dwStrLen) dwStrLen = dwStrLenActual;
	}
	
	// Figure out where we need to break at the end of the string
	FT_WidthToOffsetA(ftFace, dwFontStyleFlags,
		pcszStr, dwStrLen, dwMaxWidth, NULL, &dwTruncWidth, &dwOffset);
		
	if (dwOffset < dwStrLen)
	{
		// If not even an ellipsis will fit, draw nothing.
		if (dwEllipsisWidth > dwMaxWidth)
			return;
			
		// If ellipsis would get truncated, shorten string again
		if (dwTruncWidth+dwEllipsisWidth > dwMaxWidth)
		{
			// Figure out string break point again
			FT_WidthToOffsetA(ftFace, dwFontStyleFlags, pcszStr, dwStrLen,
				dwMaxWidth-dwEllipsisWidth, NULL, &dwTruncWidth, &dwOffset);
		}
		
		// Draw the text
		_FT_DrawTextA(ftLibrary, ftFace, ftCache, dwFontStyleFlags,
			destX, destY, pcszStr, dwOffset, pDestBuffDesc, pClipRect,
			dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor, dwShadowOpacity);
			
		// Draw the ellipsis
		_FT_DrawTextA(ftLibrary, ftFace, ftCache, dwFontStyleFlags,
			destX+dwTruncWidth, destY, szEllipsisStr, MGLSTRLEN_INFINITE, pDestBuffDesc, pClipRect,
			dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor, dwShadowOpacity);
	}
	else
	{
		// Draw the text normally
		_FT_DrawTextA(ftLibrary, ftFace, ftCache, dwFontStyleFlags,
			destX, destY, pcszStr, dwStrLen, pDestBuffDesc, pClipRect,
			dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor, dwShadowOpacity);
	}
}

static
void _FT_DrawTextEllipsisW(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	LONG destX, LONG destY, const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth,
	MGLBUFFDESC *pDestBuffDesc, RECT *pClipRect, COLORREF dwColor, DWORD dwDrawTextFlags,
	DWORD dwOpacity, COLORREF dwShadowColor, DWORD dwShadowOpacity)
{
	const WCHAR wszEllipsisStr[] = {'.', '.', '.', '\0'};
	DWORD dwEllipsisWidth;
	DWORD dwStrLenActual;
	DWORD dwTruncWidth, dwOffset;
	
	// Get width of ellipsis string
	FT_ComputeBBoxW(ftFace, dwFontStyleFlags, wszEllipsisStr, MGLSTRLEN_INFINITE,
		&dwEllipsisWidth, NULL);
	
	// Compute length of the string. This helps us keep the code
	// cleaner, especially so we don't have to have special handling
	// for the case when (dwStrLen == MGLSTRLEN_INFINITE).
	if (dwStrLen == MGLSTRLEN_INFINITE)
	{
		dwStrLenActual = S_StrLenW(pcwszStr);
		if (dwStrLenActual < dwStrLen) dwStrLen = dwStrLenActual;
	}
	
	// Figure out where we need to break at the end of the string
	FT_WidthToOffsetW(ftFace, dwFontStyleFlags,
		pcwszStr, dwStrLen, dwMaxWidth, NULL, &dwTruncWidth, &dwOffset);
		
	if (dwOffset < dwStrLen)
	{
		// If not even an ellipsis will fit, draw nothing.
		if (dwEllipsisWidth > dwMaxWidth)
			return;
			
		// If ellipsis would get truncated, shorten string again
		if (dwTruncWidth+dwEllipsisWidth > dwMaxWidth)
		{
			// Figure out string break point again
			FT_WidthToOffsetW(ftFace, dwFontStyleFlags, pcwszStr, dwStrLen,
				dwMaxWidth-dwEllipsisWidth, NULL, &dwTruncWidth, &dwOffset);
		}
		
		// Draw the text
		_FT_DrawTextW(ftLibrary, ftFace, ftCache, dwFontStyleFlags,
			destX, destY, pcwszStr, dwOffset, pDestBuffDesc, pClipRect,
			dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor, dwShadowOpacity);
			
		// Draw the ellipsis
		_FT_DrawTextW(ftLibrary, ftFace, ftCache, dwFontStyleFlags,
			destX+dwTruncWidth, destY, wszEllipsisStr, MGLSTRLEN_INFINITE, pDestBuffDesc, pClipRect,
			dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor, dwShadowOpacity);
	}
	else
	{
		// Draw the text normally
		_FT_DrawTextW(ftLibrary, ftFace, ftCache, dwFontStyleFlags,
			destX, destY, pcwszStr, dwStrLen, pDestBuffDesc, pClipRect,
			dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor, dwShadowOpacity);
	}
}

// Draws the specified ANSI text (extended).
// Returns: Height of the text.
DWORD FT_DrawTextExA(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	RECT *pRect, const CHAR *pcszStr, DWORD dwStrLen, MGLBUFFDESC *pDestBuffDesc, RECT *pClipRect,
	COLORREF dwColor, DWORD dwFlags, COLORREF dwBkColor, DWORD dwOpacity, DWORD dwBkOpacity,
	COLORREF dwShadowColor, DWORD dwShadowOpacity)
{
	LONG nTextLineHeight;
	DWORD dwTextWidth;
	DWORD dwDrawTextFlags;
	LONG x, y;
	DWORD dwMaxWidth;
	DWORD dwDrawnTextHeight;
	
	// ---------------------------------
	// Init flags for calling the original DrawText()
	// ---------------------------------
	dwDrawTextFlags = 0;
	
	if (dwFlags & MGLDRAWTEXT_EX_OPACITY)
		dwDrawTextFlags |= MGLDRAWTEXT_OPACITY;
		
	if (dwFlags & MGLDRAWTEXT_EX_SHADOW)
		dwDrawTextFlags |= MGLDRAWTEXT_SHADOW;
		
	// ---------------------------------
	// Get text metrics
	// ---------------------------------
	FT_GetTextMetrics(ftFace, NULL, NULL, &nTextLineHeight, NULL);
	
	// ---------------------------------
	// Init draw values
	// ---------------------------------
	x = pRect->left;
	y = pRect->top;
	dwMaxWidth = pRect->right-pRect->left;
	
	// Compute length of the string. This helps us keep the code
	// cleaner, especially so we don't have to have special handling
	// for the case when (dwStrLen == MGLSTRLEN_INFINITE).
	if (dwStrLen == MGLSTRLEN_INFINITE)
	{
		DWORD dwStrLenActual = S_StrLenA(pcszStr);
		if (dwStrLenActual < dwStrLen) dwStrLen = dwStrLenActual;
	}
	
	// ---------------------------------
	// Fill background, if needed
	// ---------------------------------
	if (dwFlags & MGLDRAWTEXT_EX_BKFILL)
	{
		RECT fillRect;
		LONG nFillRectWidth, nFillRectHeight;
		
		fillRect.left = pRect->left;
		fillRect.top = pRect->top;
		fillRect.right = pRect->right;
		fillRect.bottom = pRect->bottom;
		
		// If clipping is disabled, then make sure we
		// extend the background into the text area,
		// otherwise, things might end up looking a bit strange.
		if (dwFlags & MGLDRAWTEXT_EX_NOCLIP)
		{
			// Compute the total width of the string
			FT_ComputeBBoxA(ftFace, dwFontStyleFlags, pcszStr, dwStrLen, &dwTextWidth, NULL);
			
			// Adjust the fill rectangle
			if ((fillRect.right-fillRect.left) < (LONG)dwTextWidth)
				fillRect.right = fillRect.left+dwTextWidth;
				
			if ((fillRect.bottom-fillRect.top) < nTextLineHeight)
				fillRect.bottom = fillRect.top+nTextLineHeight;
		}
		
		// Clip the fill rectangle
		fillRect.left = max(fillRect.left, pClipRect->left);
		fillRect.top = max(fillRect.top, pClipRect->top);
		fillRect.right = min(fillRect.right, pClipRect->right);
		fillRect.bottom = min(fillRect.bottom, pClipRect->bottom);
		
		// Draw only if there's a valid fill rectangle
		if (!IsRectEmpty(&fillRect))
		{
			nFillRectWidth = fillRect.right-fillRect.left;
			nFillRectHeight = fillRect.bottom-fillRect.top;
			
			// Fill the rectangle
			if (dwFlags & MGLDRAWTEXT_EX_BKOPACITY)
			{
				_FT_FillRectOpacity(pDestBuffDesc, fillRect.left, fillRect.top,
					nFillRectWidth, nFillRectHeight, dwBkColor, dwBkOpacity);
			}
			else
			{
				_FT_FillRect(pDestBuffDesc, fillRect.left, fillRect.top,
					nFillRectWidth, nFillRectHeight, dwBkColor);
			}
		}
	}
	
	// ---------------------------------
	// Draw the text (finally!)
	// ---------------------------------
	if (dwFlags & MGLDRAWTEXT_EX_WORDWRAP)
	{
		DWORD dwOffset, dwCurrLine, dwNumLines, dwNumCharsThisLine;
		BOOL bHasLastLine = FALSE;
		
		dwDrawnTextHeight = 0;
		dwOffset = 0; // start at beginning
		dwCurrLine = 0;
		
		// Calculate how many lines of text will fit within the given area
		dwNumLines = (pRect->bottom-pRect->top) / nTextLineHeight;
		
		// If there are multiple lines to draw, subtract one
		// so that we can draw the last line separately from the rest.
		// Doing this makes drawing truncated text easier!
		if (dwNumLines > 0)
		{
			bHasLastLine = TRUE;
			--dwNumLines;
		}
		
		// Word wrap each line while:
		//	1) We have characters to draw
		//	OR
		//	2) We haven't hit, or exceeded, the allowed draw height
		while (dwOffset < dwStrLen &&
			dwCurrLine < dwNumLines)
		{
			dwNumCharsThisLine = FT_WordWrapA(ftFace, dwFontStyleFlags,
				&pcszStr[dwOffset], dwStrLen-dwOffset, dwMaxWidth);
			
			// No more characters to draw? We're done!
			//if (!dwNumCharsThisLine) break;
			
			// Draw the line of text
			_FT_DrawTextA(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, &pcszStr[dwOffset],
				dwNumCharsThisLine, pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags,
				dwOpacity, dwShadowColor, dwShadowOpacity);
				
			// Adjust the draw position
			dwOffset += dwNumCharsThisLine;
			y += nTextLineHeight;
			++dwCurrLine;
			dwDrawnTextHeight += nTextLineHeight;
		}
		
		// Draw the last line, if there is one
		if (bHasLastLine && dwOffset < dwStrLen)
		{
			// Draw the text as truncated
			if (dwFlags & MGLDRAWTEXT_EX_WORDELLIPSIS)
			{
				_FT_DrawTextEllipsisA(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, &pcszStr[dwOffset], dwStrLen-dwOffset,
					dwMaxWidth, pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags, dwOpacity,
					dwShadowColor, dwShadowOpacity);
			}
			else
			{
				_FT_DrawTextA(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, &pcszStr[dwOffset], dwStrLen-dwOffset,
					pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor,
					dwShadowOpacity);
			}
		}
	}
	else
	{
		dwDrawnTextHeight = (DWORD)nTextLineHeight;
		
		// Draw the text as truncated
		if (dwFlags & MGLDRAWTEXT_EX_WORDELLIPSIS)
		{
			_FT_DrawTextEllipsisA(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, pcszStr, dwStrLen,
				dwMaxWidth, pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags, dwOpacity,
				dwShadowColor, dwShadowOpacity);
		}
		else
		{
			_FT_DrawTextA(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, pcszStr, dwStrLen,
				pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor,
				dwShadowOpacity);
		}
	}
	
	// Return the height of the drawn text.
	return dwDrawnTextHeight;
}

// Draws the specified Unicode text (extended).
// Returns: Height of the text.
DWORD FT_DrawTextExW(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	RECT *pRect, const WCHAR *pcwszStr, DWORD dwStrLen, MGLBUFFDESC *pDestBuffDesc, RECT *pClipRect,
	COLORREF dwColor, DWORD dwFlags, COLORREF dwBkColor, DWORD dwOpacity, DWORD dwBkOpacity,
	COLORREF dwShadowColor, DWORD dwShadowOpacity)
{
	LONG nTextLineHeight;
	DWORD dwTextWidth;
	DWORD dwDrawTextFlags;
	LONG x, y;
	DWORD dwMaxWidth;
	DWORD dwDrawnTextHeight;
	
	// ---------------------------------
	// Init flags for calling the original DrawText()
	// ---------------------------------
	dwDrawTextFlags = 0;
	
	if (dwFlags & MGLDRAWTEXT_EX_OPACITY)
		dwDrawTextFlags |= MGLDRAWTEXT_OPACITY;
		
	if (dwFlags & MGLDRAWTEXT_EX_SHADOW)
		dwDrawTextFlags |= MGLDRAWTEXT_SHADOW;
		
	// ---------------------------------
	// Get text metrics
	// ---------------------------------
	FT_GetTextMetrics(ftFace, NULL, NULL, &nTextLineHeight, NULL);
	
	// ---------------------------------
	// Init draw values
	// ---------------------------------
	x = pRect->left;
	y = pRect->top;
	dwMaxWidth = pRect->right-pRect->left;
	
	// Compute length of the string. This helps us keep the code
	// cleaner, especially so we don't have to have special handling
	// for the case when (dwStrLen == MGLSTRLEN_INFINITE).
	if (dwStrLen == MGLSTRLEN_INFINITE)
	{
		DWORD dwStrLenActual = S_StrLenW(pcwszStr);
		if (dwStrLenActual < dwStrLen) dwStrLen = dwStrLenActual;
	}
	
	// ---------------------------------
	// Fill background, if needed
	// ---------------------------------
	if (dwFlags & MGLDRAWTEXT_EX_BKFILL)
	{
		RECT fillRect;
		LONG nFillRectWidth, nFillRectHeight;
		
		fillRect.left = pRect->left;
		fillRect.top = pRect->top;
		fillRect.right = pRect->right;
		fillRect.bottom = pRect->bottom;
		
		// If clipping is disabled, then make sure we
		// extend the background into the text area,
		// otherwise, things might end up looking a bit strange.
		if (dwFlags & MGLDRAWTEXT_EX_NOCLIP)
		{
			// Compute the total width of the string
			FT_ComputeBBoxW(ftFace, dwFontStyleFlags, pcwszStr, dwStrLen, &dwTextWidth, NULL);
			
			// Adjust the fill rectangle
			if ((fillRect.right-fillRect.left) < (LONG)dwTextWidth)
				fillRect.right = fillRect.left+dwTextWidth;
				
			if ((fillRect.bottom-fillRect.top) < nTextLineHeight)
				fillRect.bottom = fillRect.top+nTextLineHeight;
		}
		
		// Clip the fill rectangle
		fillRect.left = max(fillRect.left, pClipRect->left);
		fillRect.top = max(fillRect.top, pClipRect->top);
		fillRect.right = min(fillRect.right, pClipRect->right);
		fillRect.bottom = min(fillRect.bottom, pClipRect->bottom);
		
		// Draw only if there's a valid fill rectangle
		if (!IsRectEmpty(&fillRect))
		{
			nFillRectWidth = fillRect.right-fillRect.left;
			nFillRectHeight = fillRect.bottom-fillRect.top;
			
			// Fill the rectangle
			if (dwFlags & MGLDRAWTEXT_EX_BKOPACITY)
			{
				_FT_FillRectOpacity(pDestBuffDesc, fillRect.left, fillRect.top,
					nFillRectWidth, nFillRectHeight, dwBkColor, dwBkOpacity);
			}
			else
			{
				_FT_FillRect(pDestBuffDesc, fillRect.left, fillRect.top,
					nFillRectWidth, nFillRectHeight, dwBkColor);
			}
		}
	}
	
	// ---------------------------------
	// Draw the text (finally!)
	// ---------------------------------
	if (dwFlags & MGLDRAWTEXT_EX_WORDWRAP)
	{
		DWORD dwOffset, dwCurrLine, dwNumLines, dwNumCharsThisLine;
		BOOL bHasLastLine = FALSE;
		
		dwDrawnTextHeight = 0;
		dwOffset = 0; // start at beginning
		dwCurrLine = 0;
		
		// Calculate how many lines of text will fit within the given area
		dwNumLines = (pRect->bottom-pRect->top) / nTextLineHeight;
		
		// If there are multiple lines to draw, subtract one
		// so that we can draw the last line separately from the rest.
		// Doing this makes drawing truncated text easier!
		if (dwNumLines > 0)
		{
			bHasLastLine = TRUE;
			--dwNumLines;
		}
		
		// Word wrap each line while:
		//	1) We have characters to draw
		//	OR
		//	2) We haven't hit, or exceeded, the allowed draw height
		while (dwOffset < dwStrLen &&
			dwCurrLine < dwNumLines)
		{
			dwNumCharsThisLine = FT_WordWrapW(ftFace, dwFontStyleFlags,
				&pcwszStr[dwOffset], dwStrLen-dwOffset, dwMaxWidth);
			
			// No more characters to draw? We're done!
			//if (!dwNumCharsThisLine) break;
			
			// Draw the line of text
			_FT_DrawTextW(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, &pcwszStr[dwOffset],
				dwNumCharsThisLine, pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags,
				dwOpacity, dwShadowColor, dwShadowOpacity);
				
			// Adjust the draw position
			dwOffset += dwNumCharsThisLine;
			y += nTextLineHeight;
			++dwCurrLine;
			dwDrawnTextHeight += nTextLineHeight;
		}
		
		// Draw the last line, if there is one
		if (bHasLastLine && dwOffset < dwStrLen)
		{
			// Draw the text as truncated
			if (dwFlags & MGLDRAWTEXT_EX_WORDELLIPSIS)
			{
				_FT_DrawTextEllipsisW(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, &pcwszStr[dwOffset], dwStrLen-dwOffset,
					dwMaxWidth, pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags, dwOpacity,
					dwShadowColor, dwShadowOpacity);
			}
			else
			{
				_FT_DrawTextW(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, &pcwszStr[dwOffset], dwStrLen-dwOffset,
					pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor,
					dwShadowOpacity);
			}
		}
	}
	else
	{
		dwDrawnTextHeight = (DWORD)nTextLineHeight;
		
		// Draw the text as truncated
		if (dwFlags & MGLDRAWTEXT_EX_WORDELLIPSIS)
		{
			_FT_DrawTextEllipsisW(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, pcwszStr, dwStrLen,
				dwMaxWidth, pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags, dwOpacity,
				dwShadowColor, dwShadowOpacity);
		}
		else
		{
			_FT_DrawTextW(ftLibrary, ftFace, ftCache, dwFontStyleFlags, x, y, pcwszStr, dwStrLen,
				pDestBuffDesc, pClipRect, dwColor, dwDrawTextFlags, dwOpacity, dwShadowColor,
				dwShadowOpacity);
		}
	}
	
	// Return the height of the drawn text.
	return dwDrawnTextHeight;
}