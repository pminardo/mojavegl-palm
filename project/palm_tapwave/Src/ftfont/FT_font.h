/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __FT_FONT_H_
#define __FT_FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* Font Functions
**************************************************************/

// Initialize the FreeType library.
// Returns: FreeType error code.
FTERR FT_OpenLibrary(FTLIBRARY *ftLibrary /*OUT*/);

// Close the FreeType library instance.
// Returns: FreeType error code.
FTERR FT_CloseLibrary(FTLIBRARY ftLibrary);

// Create a font face from a memory buffer.
// Returns: FreeType error code.
FTERR FT_CreateFontFaceMemory(FTLIBRARY ftLibrary, const BYTE *pFontData, DWORD dwFontDataSize, LONG nFaceIndex,
	DWORD dwCharWidth, DWORD dwCharHeight, DWORD dwHorizDpi, DWORD dwVertDpi,
	FTFACE *ftFace/*OUT*/, FTGLYPHCACHE *ftGlyphCache/*OUT*/, DWORD dwGlyphCacheMaxItems, LONG *pnAscent /*OUT*/, LONG *pnDescent/*OUT*/, LONG *pnHeight/*OUT*/, LONG *pnMaxHorizAdvance/*OUT*/);

// Set the font face character size.
// Returns: FreeType error code.
FTERR FT_SetFontFaceCharSize(FTFACE ftFace, DWORD dwCharWidth, DWORD dwCharHeight, DWORD dwHorizDpi, DWORD dwVertDpi,
	LONG *pnAscent /*OUT*/, LONG *pnDescent/*OUT*/, LONG *pnHeight/*OUT*/, LONG *pnMaxHorizAdvance/*OUT*/);

// Retrieves text metrics for the specified font face.
void FT_GetTextMetrics(FTFACE ftFace, LONG *pnAscent /*OUT*/, LONG *pnDescent/*OUT*/,
	LONG *pnHeight/*OUT*/, LONG *pnMaxHorizAdvance/*OUT*/);

// Computes bounding box of the specified ANSI string.
void FT_ComputeBBoxA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD *pdwWidth /*OUT*/, DWORD *pdwHeight /*OUT*/);

// Computes bounding box of the specified Unicode string.
void FT_ComputeBBoxW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD *pdwWidth /*OUT*/, DWORD *pdwHeight /*OUT*/);

// Computes the line bounding box of the specified ANSI string.
void FT_ComputeLineBBoxA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD *pdwWidth /*OUT*/, DWORD *pdwHeight /*OUT*/);

// Computes the line bounding box of the specified Unicode string.
void FT_ComputeLineBBoxW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD *pdwWidth /*OUT*/, DWORD *pdwHeight /*OUT*/);

// Performs word wrapping for the specified ANSI string.
DWORD FT_WordWrapA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth);

// Performs word wrapping for the specified Unicode string.
DWORD FT_WordWrapW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth);
	
// Computes scroll values for the specified ANSI string.
void FT_GetScrollValuesA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwWidth, DWORD dwScrollPos,
	DWORD *pdwLines /*OUT*/, DWORD *pdwTopLine /*OUT*/);
	
// Computes scroll values for the specified Unicode string.
void FT_GetScrollValuesW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwWidth, DWORD dwScrollPos,
	DWORD *pdwLines /*OUT*/, DWORD *pdwTopLine /*OUT*/);
	
// Performs reverse word wrapping for the specified ANSI string.
void FT_WordWrapReverseNLinesA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwMaxWidth, DWORD *pdwLinesToScroll, DWORD *pdwScrollPos);
	
// Performs reverse word wrapping for the specified Unicode string.
void FT_WordWrapReverseNLinesW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwMaxWidth, DWORD *pdwLinesToScroll, DWORD *pdwScrollPos);

// Performs text formatting for the specified ANSI string.
MGLTEXTLINENODE* FT_FormatTextA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth);

// Performs text formatting for the specified Unicode string.
MGLTEXTLINENODE* FT_FormatTextW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth);

// Frees the specified MGLTEXTLINENODE linked list.
void FT_DoneFormatText(MGLTEXTLINENODE *pHead);

// Converts the specified width to an offset in the specified ANSI string.
void FT_WidthToOffsetA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, DWORD dwWidth,
	BOOL *pbLeadingEdge /*OUT*/, DWORD *pdwTruncWidth /*OUT*/, DWORD *pdwOffset /*OUT*/);

// Converts the specified width to an offset in the specified Unicode string.
void FT_WidthToOffsetW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwWidth,
	BOOL *pbLeadingEdge /*OUT*/, DWORD *pdwTruncWidth /*OUT*/, DWORD *pdwOffset /*OUT*/);

// Determines how many characters in the specified ANSI string fit within a given width.
void FT_CharsInWidthA(FTFACE ftFace, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD *pdwStrWidth /*IN & OUT*/,
	DWORD *pdwStrLen /*IN & OUT*/, BOOL *pbFitWithinWidth /*OUT*/);

// Determines how many characters in the specified Unicode string fit within a given width.
void FT_CharsInWidthW(FTFACE ftFace, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD *pdwStrWidth /*IN & OUT*/,
	DWORD *pdwStrLen /*IN & OUT*/, BOOL *pbFitWithinWidth /*OUT*/);

// Delete a FreeType font face and a glyph cache (if non-NULL).
// Returns: FreeType error code.
FTERR FT_DeleteFontFace(FTFACE ftFace, FTGLYPHCACHE ftGlyphCache);

/**************************************************************
* Text Drawing Functions
**************************************************************/

// Draws ANSI text (RGB565).
void FT_DrawTextA_565(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	const CHAR *pcszStr, DWORD dwStrLen, void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, LONG destWidth, LONG destHeight, RECT *pClipRect, DWORD dwNativeColor,
	DWORD dwFlags, DWORD dwOpacity, DWORD dwNativeShadowColor, DWORD dwShadowOpacity);

// Draws Unicode text (RGB565).
void FT_DrawTextW_565(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	const WCHAR *pcwszStr, DWORD dwStrLen, void *dest, LONG destXPitch, LONG destYPitch,
	LONG destX, LONG destY, LONG destWidth, LONG destHeight, RECT *pClipRect, DWORD dwNativeColor,
	DWORD dwFlags, DWORD dwOpacity, DWORD dwNativeShadowColor, DWORD dwShadowOpacity);

// Draws the specified ANSI text (extended).
// Returns: Height of the text.
DWORD FT_DrawTextExA(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	RECT *pRect, const CHAR *pcszStr, DWORD dwStrLen, MGLBUFFDESC *pDestBuffDesc, RECT *pClipRect,
	COLORREF dwColor, DWORD dwFlags, COLORREF dwBkColor, DWORD dwOpacity, DWORD dwBkOpacity,
	COLORREF dwShadowColor, DWORD dwShadowOpacity);
	
// Draws the specified Unicode text (extended).
// Returns: Height of the text.
DWORD FT_DrawTextExW(FTLIBRARY ftLibrary, FTFACE ftFace, FTGLYPHCACHE ftCache, DWORD dwFontStyleFlags,
	RECT *pRect, const WCHAR *pcwszStr, DWORD dwStrLen, MGLBUFFDESC *pDestBuffDesc, RECT *pClipRect,
	COLORREF dwColor, DWORD dwFlags, COLORREF dwBkColor, DWORD dwOpacity, DWORD dwBkOpacity,
	COLORREF dwShadowColor, DWORD dwShadowOpacity);

#ifdef __cplusplus
}
#endif

#endif // __FT_FONT_H_