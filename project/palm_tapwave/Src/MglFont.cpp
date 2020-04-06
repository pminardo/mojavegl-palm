/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"

#include "ftfont/FT_defines.h"
#include "ftfont/FT_font.h"

/**************************************************************
* Constants/Macros
**************************************************************/

#define MGL_FONT_FTR_CREATOR		'MGLf'

// The following constant controls the default font DPI
#define MGL_FONT_DEFAULT_DPI		72

#define MGL_FONT_TINYCACHE_MAXITEMS	32

/**************************************************************
* Globals
**************************************************************/

static WORD g_wNextFontFtrNum = 0;

/**************************************************************
* Internal Functions
**************************************************************/

// CMglFont constructor
CMglFont::CMglFont(CMgl* pMgl)
{
	m_pMgl = pMgl;
	
	// Clear font internals
	mgl_memset(&m_font, 0, sizeof(mglinternal::MGLFONT));
}

// CMglFont destructor
CMglFont::~CMglFont()
{
	// Free resources allocated by the font
	FreeFont();
}

HRESULT CMglFont::CreateFont(DWORD dwFlags, DWORD dwStyle, DWORD dwCharWidth, DWORD dwCharHeight, DWORD dwHorizDpi, DWORD dwVertDpi, BYTE *pFontFileMem, DWORD dwFontFileSize, LONG nFaceIndex)
{
	Err err;
	void *pBuffer;
	DWORD dwFtrCreatorId;
	LONG nFtrNum;
	FTLIBRARY ftLibrary;
	FTERR ftError;
	FTFACE ftFace;
	FTGLYPHCACHE ftGlyphCache = NULL;
	FTGLYPHCACHE *ppFtGlyphCache;
	DWORD dwGlyphCacheMaxItems;
	LONG nAscent, nDescent, nHeight, nMaxHorizAdvance;
	HRESULT hr;

	// Verify the MGL instance
	if (!m_pMgl->IsInitialized())
		return MGLERR_INVALIDINSTANCE;

	// If the font is already created, bail out.
	if (m_font.pFtFace) return MGLERR_ALREADYINITIALIZED;
	
	// Verify parameters
	if (!pFontFileMem || dwFontFileSize == 0)
		return MGLERR_INVALIDPARAMS;
		
	// Adjust DPIs, if needed
	if (!dwHorizDpi) dwHorizDpi = MGL_FONT_DEFAULT_DPI;
	if (!dwVertDpi) dwVertDpi = MGL_FONT_DEFAULT_DPI;
	
	// Init glyph cache values 
	if (dwFlags & MGLFONT_NOCACHE)
	{
		dwGlyphCacheMaxItems = 0;
		ppFtGlyphCache = NULL;
	}
	else
	{
		dwGlyphCacheMaxItems = (dwFlags & MGLFONT_TINYCACHE) ?
			MGL_FONT_TINYCACHE_MAXITEMS : 0xFFFFFFFFL;
		ppFtGlyphCache = &ftGlyphCache;
	}
	
	// If we need to make a copy of the font data, do it now.
	if (dwFlags & MGLFONT_LOCALCOPY)
	{
		if (dwFlags & MGLFONT_READONLY)
		{
			// Allocate in storage heap
			err = FtrPtrNew(MGL_FONT_FTR_CREATOR,
				g_wNextFontFtrNum,
				dwFontFileSize,
				&pBuffer);
			
			// Check for error
			if (err != errNone)
			{
				if (err == memErrNotEnoughSpace)
					return MGLERR_OUTOFMEMORY;
				else
					return MGLERR_INVALIDPARAMS;
			}
			
			// Copy font data
			DmWrite(pBuffer, 0, pFontFileMem, dwFontFileSize);
			
			// Store ftr memory-specific stuff
			dwFtrCreatorId = MGL_FONT_FTR_CREATOR;
			nFtrNum = g_wNextFontFtrNum;
		}
		else
		{
			// Allocate in the dynamic heap
			pBuffer = mgl_malloc(dwFontFileSize);
			if (!pBuffer) return MGLERR_OUTOFMEMORY;
			
			// Copy font data
			mgl_memcpy(pBuffer, pFontFileMem, dwFontFileSize);
		}
	}
	else
	{
		// Use the provided font file (in memory) as the data buffer
		pBuffer = pFontFileMem;
	}
	
	// Get the FreeType library pointer from the base class
	ftLibrary = (FTLIBRARY)m_pMgl->m_globals.pFtLibrary;
	
	// Create the new font from the font data buffer
	ftError = FT_CreateFontFaceMemory(ftLibrary, (const BYTE*)pBuffer, dwFontFileSize, nFaceIndex,
		dwCharWidth, dwCharHeight, dwHorizDpi, dwVertDpi, &ftFace,
		ppFtGlyphCache, dwGlyphCacheMaxItems, &nAscent, &nDescent, &nHeight, &nMaxHorizAdvance);
	
	if (ftError != FT_OK)
	{
		// Translate the FreeType error
		switch (ftError)
		{
			case FTERR_INVALID_ARGUMENT:
			
			/* Handle errors */
			case FTERR_INVALID_HANDLE:
			case FTERR_INVALID_LIBRARY_HANDLE:
			case FTERR_INVALID_DRIVER_HANDLE:
			case FTERR_INVALID_FACE_HANDLE:
			case FTERR_INVALID_SIZE_HANDLE:
			case FTERR_INVALID_SLOT_HANDLE:
			case FTERR_INVALID_CHARMAP_HANDLE:
			case FTERR_INVALID_CACHE_HANDLE:
			case FTERR_INVALID_STREAM_HANDLE:
			
				hr = MGLERR_INVALIDPARAMS;
				break;
		
			case FTERR_OUT_OF_MEMORY:
				hr = MGLERR_OUTOFMEMORY;
				break;
				
			case FTERR_CANNOT_OPEN_RESOURCE:
			case FTERR_UNKNOWN_FILE_FORMAT:
			case FTERR_INVALID_FILE_FORMAT:
			
			/* stream errors */
			case FTERR_CANNOT_OPEN_STREAM:
			case FTERR_INVALID_STREAM_SEEK:
			case FTERR_INVALID_STREAM_SKIP:
			case FTERR_INVALID_STREAM_READ:
			case FTERR_INVALID_STREAM_OPERATION:
			case FTERR_INVALID_FRAME_OPERATION:
			case FTERR_NESTED_FRAME_ACCESS:
			case FTERR_INVALID_FRAME_READ:
			
			/* TrueType and SFNT errors */
			case FTERR_INVALID_OPCODE:
			case FTERR_TOO_FEW_ARGUMENTS:
			case FTERR_STACK_OVERFLOW:
			case FTERR_CODE_OVERFLOW:
			case FTERR_BAD_ARGUMENT:
			case FTERR_DIVIDE_BY_ZERO:
			case FTERR_INVALID_REFERENCE:
			case FTERR_DEBUG_OPCODE:
			case FTERR_ENDF_IN_EXEC_STREAM:
			case FTERR_NESTED_DEFS:
			case FTERR_INVALID_CODERANGE:
			case FTERR_EXECUTION_TOO_LONG:
			case FTERR_TOO_MANY_FUNCTION_DEFS:
			case FTERR_TOO_MANY_INSTRUCTION_DEFS:
			case FTERR_TABLE_MISSING:
			case FTERR_HORIZ_HEADER_MISSING:
			case FTERR_LOCATIONS_MISSING:
			case FTERR_NAME_TABLE_MISSING:
			case FTERR_CMAP_TABLE_MISSING:
			case FTERR_HMTX_TABLE_MISSING:
			case FTERR_POST_TABLE_MISSING:
			
			/* CFF, CID, and Type 1 errors */
			case FTERR_SYNTAX_ERROR:
			case FTERR_STACK_UNDERFLOW:
			case FTERR_IGNORE:
			
			/* BDF errors */
			case FTERR_MISSING_STARTFONT_FIELD:
			case FTERR_MISSING_FONT_FIELD:
			case FTERR_MISSING_SIZE_FIELD:
			case FTERR_MISSING_CHARS_FIELD:
			case FTERR_MISSING_STARTCHAR_FIELD:
			case FTERR_MISSING_ENCODING_FIELD:
			case FTERR_MISSING_BBX_FIELD:
			
				hr = MGLERR_INVALIDFONT;
				break;
				
			default:
				hr = E_FAIL;
				break;
		}
	
		goto error;
	}
	
	// -------------------------------------
	// Init font internals
	// -------------------------------------
	
	m_font.dwFlags = dwFlags;
	m_font.dwStyle = dwStyle;
	m_font.dwCharWidth = dwCharWidth;
	m_font.dwCharHeight = dwCharHeight;
	m_font.dwHorizDpi = dwHorizDpi;
	m_font.dwVertDpi = dwVertDpi;
	m_font.nAscent = nAscent;
	m_font.nDescent = nDescent;
	m_font.nHeight = nHeight;
	m_font.nMaxHorizAdvance = nMaxHorizAdvance;
	m_font.pFtFace = ftFace;
	m_font.pBuffer = pBuffer;
	m_font.pGlyphCache = ftGlyphCache;
	
	// Store ftr memory-specific stuff if the font data
	// buffer was allocated in ftr memory.
	if ((dwFlags & MGLFONT_LOCALCOPY) &&
		(dwFlags & MGLFONT_READONLY))
	{
		m_font.dwCreatorID = dwFtrCreatorId;
		m_font.ftrNum = nFtrNum;
	
		// Adjust font ftr num
		++g_wNextFontFtrNum;
	}
	
	return MGL_OK;
	
	error:
	
	// -----------------------------------------------
	// Free previously allocated memory
	// -----------------------------------------------
	if (dwFlags & MGLFONT_LOCALCOPY)
	{
		if (dwFlags & MGLFONT_READONLY)
		{
			// Free memory allocated from storage heap
			FtrPtrFree(dwFtrCreatorId, (WORD)nFtrNum);
		}
		else
		{
			// Free memory allocated from the dynamic heap
			mgl_free(pBuffer);
		}
	}
	
	return hr;
}

HRESULT CMglFont::FreeFont()
{
	FTFACE ftFace;
	FTGLYPHCACHE ftGlyphCache;

	// If there's nothing to free, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ----------------------------------
	// Free font data buffer
	// ----------------------------------
	
	// If we need to make a copy of the font data, do it now.
	if (m_font.dwFlags & MGLFONT_LOCALCOPY)
	{
		if (m_font.dwFlags & MGLFONT_READONLY)
		{
			// Free font data buffer stored in storage memory
			FtrPtrFree(m_font.dwCreatorID, (WORD)m_font.ftrNum);
		}
		else
		{
			// Free font data buffer stored in dynamic memory
			mgl_free(m_font.pBuffer);
		}
	}
	
	// ----------------------------------
	// Delete font face
	// ----------------------------------
	ftFace = (FTFACE)m_font.pFtFace;
	ftGlyphCache = (FTGLYPHCACHE)m_font.pGlyphCache;
	FT_DeleteFontFace(ftFace, ftGlyphCache);
	
	// Clear font internals
	mgl_memset(&m_font, 0, sizeof(mglinternal::MGLFONT));
	
	return MGL_OK;
}

// Returns the font flags.
DWORD CMglFont::GetFontFlags()
{
	return m_font.dwFlags;
}

// Sets the current character size. The font character size
// is computed based on the specified DPI. Pass 0 for the
// DPI parameters to use the default values.
HRESULT CMglFont::SetCharSize(DWORD dwCharWidth, DWORD dwCharHeight, DWORD dwHorizDpi, DWORD dwVertDpi)
{
	FTERR ftError;
	FTFACE ftFace;
	LONG nAscent, nDescent, nHeight, nMaxHorizAdvance;
	HRESULT hr;
	
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// Set font face character size
	ftFace = (FTFACE)m_font.pFtFace;
	ftError = FT_SetFontFaceCharSize(ftFace, dwCharWidth, dwCharHeight, dwHorizDpi, dwVertDpi,
		&nAscent, &nDescent, &nHeight, &nMaxHorizAdvance);
		
	if (ftError != FT_OK)
	{
		// Translate the FreeType error
		switch (ftError)
		{
			case FTERR_INVALID_ARGUMENT:
			
			/* Handle errors */
			case FTERR_INVALID_HANDLE:
			case FTERR_INVALID_LIBRARY_HANDLE:
			case FTERR_INVALID_DRIVER_HANDLE:
			case FTERR_INVALID_FACE_HANDLE:
			case FTERR_INVALID_SIZE_HANDLE:
			case FTERR_INVALID_SLOT_HANDLE:
			case FTERR_INVALID_CHARMAP_HANDLE:
			case FTERR_INVALID_CACHE_HANDLE:
			case FTERR_INVALID_STREAM_HANDLE:
				hr = MGLERR_INVALIDPARAMS;
				break;
		
			case FTERR_OUT_OF_MEMORY:
				hr = MGLERR_OUTOFMEMORY;
				break;
			
			default:
				hr = E_FAIL;
				break;
		}
	
		return hr;	// bail out
	}

	// Update text metrics
	m_font.nAscent = nAscent;
	m_font.nDescent = nDescent;
	m_font.nHeight = nHeight;
	m_font.nMaxHorizAdvance = nMaxHorizAdvance;
	
	return MGL_OK;	
}

// Returns the current font style.
DWORD CMglFont::GetStyle()
{
	return m_font.dwStyle;
}

// Sets the current font style. The return
// value is the previous font style.
DWORD CMglFont::SetStyle(DWORD dwStyle)
{
	DWORD dwPreviousStyle;
	
	dwPreviousStyle = m_font.dwStyle;
	
	// Set the new font style
	m_font.dwStyle = dwStyle;
	
	return dwPreviousStyle;
}

// Retrieves the font's text metrics.
HRESULT CMglFont::GetTextMetrics(MGLTEXTMETRICS *pTextMetrics)
{
	// Verify parameters
	if (!pTextMetrics)
		return MGLERR_INVALIDPARAMS;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;

	// Return text metrics
	pTextMetrics->nAscent = m_font.nAscent;
	pTextMetrics->nDescent = m_font.nDescent;
	pTextMetrics->nHeight = m_font.nHeight;
	pTextMetrics->nMaxHorizAdvance = m_font.nMaxHorizAdvance;
		
	return MGL_OK;
}

HRESULT CMglFont::GetStrBBox(const CHAR *pcszStr, DWORD dwStrLen, MGLBBOX *pBBox)
{
	FTFACE ftFace;
	DWORD dwWidth, dwHeight;

	// Verify parameters
	if (!pcszStr || !pBBox)
		return MGLERR_INVALIDPARAMS;

	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
		
	// ------------------------------------------------
	// Compute bounding box
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_ComputeBBoxA(ftFace, m_font.dwStyle, pcszStr, dwStrLen,
		&dwWidth, &dwHeight);
	
	pBBox->dwWidth = dwWidth;
	pBBox->dwHeight = dwHeight;
		
	return MGL_OK;
}

// Computes the bounding box of the specified text string.
// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string.
HRESULT CMglFont::GetStrBBox(const WCHAR *pcwszStr, DWORD dwStrLen, MGLBBOX *pBBox)
{
	FTFACE ftFace;
	DWORD dwWidth, dwHeight;

	// Verify parameters
	if (!pcwszStr || !pBBox)
		return MGLERR_INVALIDPARAMS;

	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
		
	// ------------------------------------------------
	// Compute bounding box
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_ComputeBBoxW(ftFace, m_font.dwStyle, pcwszStr, dwStrLen,
		&dwWidth, &dwHeight);
	
	pBBox->dwWidth = dwWidth;
	pBBox->dwHeight = dwHeight;
		
	return MGL_OK;
}

// Computes the width of the specified text string.
// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string.
DWORD CMglFont::GetStrWidth(const CHAR *pcszStr, DWORD dwStrLen)
{
	FTFACE ftFace;
	DWORD dwWidth;
	
	// Verify parameters
	if (!pcszStr) return 0;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return 0;
	
	// Optimization: Don't waste time retrieving width if there's no text
	if (dwStrLen == 0) return 0;
	
	// ------------------------------------------------
	// Compute string width
	// ------------------------------------------------

	ftFace = (FTFACE)m_font.pFtFace;
	FT_ComputeBBoxA(ftFace, m_font.dwStyle, pcszStr, dwStrLen, &dwWidth, NULL);

	return dwWidth;
}

// Computes the width of the specified text string.
// Pass MGLSTRLEN_INFINITE for dwStrLen to use the entire string.
DWORD CMglFont::GetStrWidth(const WCHAR *pcwszStr, DWORD dwStrLen)
{
	FTFACE ftFace;
	DWORD dwWidth;
	
	// Verify parameters
	if (!pcwszStr) return 0;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return 0;
	
	// Optimization: Don't waste time retrieving width if there's no text
	if (dwStrLen == 0) return 0;
		
	// ------------------------------------------------
	// Compute string width
	// ------------------------------------------------

	ftFace = (FTFACE)m_font.pFtFace;
	FT_ComputeBBoxW(ftFace, m_font.dwStyle, pcwszStr, dwStrLen, &dwWidth, NULL);

	return dwWidth;
}

DWORD CMglFont::GetLineWidth(const CHAR *pcszStr, DWORD dwStrLen)
{
	FTFACE ftFace;
	DWORD dwWidth;
	
	// Verify parameters
	if (!pcszStr) return 0;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return 0;
	
	// Optimization: Don't waste time retrieving width if there's no text
	if (dwStrLen == 0) return 0;
		
	// ------------------------------------------------
	// Compute line width
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_ComputeLineBBoxA(ftFace, m_font.dwStyle, pcszStr, dwStrLen, &dwWidth, NULL);

	return dwWidth;
}

DWORD CMglFont::GetLineWidth(const WCHAR *pcwszStr, DWORD dwStrLen)
{
	FTFACE ftFace;
	DWORD dwWidth;
	
	// Verify parameters
	if (!pcwszStr) return 0;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return 0;
	
	// Optimization: Don't waste time retrieving width if there's no text
	if (dwStrLen == 0) return 0;
		
	// ------------------------------------------------
	// Compute line width
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_ComputeLineBBoxW(ftFace, m_font.dwStyle, pcwszStr, dwStrLen, &dwWidth, NULL);

	return dwWidth;
}

HRESULT CMglFont::GetScrollValues(const CHAR *pcszStr, DWORD dwWidth, DWORD dwScrollPos, DWORD *pdwLines /*OUT*/, DWORD *pdwTopLine /*OUT*/)
{
	FTFACE ftFace;
	
	// Verify parameters
	if (!pcszStr || !pdwLines || !pdwTopLine)
		return MGLERR_INVALIDPARAMS;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Compute scroll values
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_GetScrollValuesA(ftFace, m_font.dwStyle, pcszStr, dwWidth, dwScrollPos,
		pdwLines, pdwTopLine);

	return MGL_OK;
}

HRESULT CMglFont::GetScrollValues(const WCHAR *pcwszStr, DWORD dwWidth, DWORD dwScrollPos, DWORD *pdwLines /*OUT*/, DWORD *pdwTopLine /*OUT*/)
{
	FTFACE ftFace;
	
	// Verify parameters
	if (!pcwszStr || !pdwLines || !pdwTopLine)
		return MGLERR_INVALIDPARAMS;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Compute scroll values
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_GetScrollValuesW(ftFace, m_font.dwStyle, pcwszStr, dwWidth, dwScrollPos,
		pdwLines, pdwTopLine);
	
	return MGL_OK;
}

DWORD CMglFont::WordWrap(const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth)
{
	FTFACE ftFace;
	DWORD dwLineWidth;
	
	// Verify parameters
	if (!pcszStr) return 0;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return 0;
	
	// Optimization: Don't waste time word wrapping if there's no text
	if (dwStrLen == 0) return 0;
	
	// Optimization: Don't waste time word wrapping if dwMaxWidth
	// is some ridiculous value.
	if (dwMaxWidth == 0) return 0;
	
	// ------------------------------------------------
	// Perform word wrapping calculations
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	dwLineWidth = FT_WordWrapA(ftFace, m_font.dwStyle, pcszStr, dwStrLen, dwMaxWidth);
	
	return dwLineWidth;
}

DWORD CMglFont::WordWrap(const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth)
{
	FTFACE ftFace;
	DWORD dwLineWidth;
	
	// Verify parameters
	if (!pcwszStr) return 0;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return 0;
	
	// Optimization: Don't waste time word wrapping if there's no text
	if (dwStrLen == 0) return 0;
	
	// Optimization: Don't waste time word wrapping if dwMaxWidth
	// is some ridiculous value.
	if (dwMaxWidth == 0) return 0;
	
	// ------------------------------------------------
	// Perform word wrapping calculations
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	dwLineWidth = FT_WordWrapW(ftFace, m_font.dwStyle, pcwszStr, dwStrLen, dwMaxWidth);
	
	return dwLineWidth;
}

HRESULT CMglFont::WordWrapReverseNLines(const CHAR *pcszStr, DWORD dwMaxWidth, DWORD *pdwLinesToScroll /*IN & OUT*/, DWORD *pdwScrollPos /*IN & OUT*/)
{
	FTFACE ftFace;
	
	// Verify parameters
	if (!pcszStr || !pdwLinesToScroll || !pdwScrollPos)
		return MGLERR_INVALIDPARAMS;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Perform reverse word wrapping calculations
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_WordWrapReverseNLinesA(ftFace, m_font.dwStyle,
		pcszStr, dwMaxWidth, pdwLinesToScroll, pdwScrollPos);
	
	return MGL_OK;
}

HRESULT CMglFont::WordWrapReverseNLines(const WCHAR *pcwszStr, DWORD dwMaxWidth, DWORD *pdwLinesToScroll /*IN & OUT*/, DWORD *pdwScrollPos /*IN & OUT*/)
{
	FTFACE ftFace;
	
	// Verify parameters
	if (!pcwszStr || !pdwLinesToScroll || !pdwScrollPos)
		return MGLERR_INVALIDPARAMS;
	
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Perform reverse word wrapping calculations
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_WordWrapReverseNLinesW(ftFace, m_font.dwStyle,
		pcwszStr, dwMaxWidth, pdwLinesToScroll, pdwScrollPos);
	
	return MGL_OK;
}

HRESULT CMglFont::FormatText(const CHAR *pcszStr, DWORD dwStrLen, DWORD dwMaxWidth, MGLTEXTLINENODE **ppTextLineHead)
{
	FTFACE ftFace;
	MGLTEXTLINENODE *pHead;
	
	// Verify parameters
	if (!pcszStr || !ppTextLineHead)
		return MGLERR_INVALIDPARAMS;
	
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Perform text formatting
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	pHead = FT_FormatTextA(ftFace, m_font.dwStyle, pcszStr, dwStrLen, dwMaxWidth);
	
	// Check for an error. (FT_FormatText returns NULL
	// only when memory allocations have failed.)
	if (!pHead)
		return MGLERR_OUTOFMEMORY;
	
	// Return values
	*ppTextLineHead = pHead;
	
	return MGL_OK;
}

HRESULT CMglFont::FormatText(const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwMaxWidth, MGLTEXTLINENODE **ppTextLineHead)
{
	FTFACE ftFace;
	MGLTEXTLINENODE *pHead;
	
	// Verify parameters
	if (!pcwszStr || !ppTextLineHead)
		return MGLERR_INVALIDPARAMS;
	
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Perform text formatting
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	pHead = FT_FormatTextW(ftFace, m_font.dwStyle, pcwszStr, dwStrLen, dwMaxWidth);
	
	// Check for an error. (FT_FormatText returns NULL
	// only when memory allocations have failed.)
	if (!pHead)
		return MGLERR_OUTOFMEMORY;
	
	// Return values
	*ppTextLineHead = pHead;
	
	return MGL_OK;
}

HRESULT CMglFont::DoneFormatText(MGLTEXTLINENODE *pTextLineHead)
{
	// Verify parameters
	if (!pTextLineHead)
		return MGLERR_INVALIDPARAMS;
		
	// ------------------------------------------------
	// Free the MGLTEXTLINENODE linked list
	// ------------------------------------------------
	FT_DoneFormatText(pTextLineHead);

	return MGL_OK;
}

HRESULT CMglFont::WidthToOffset(const CHAR *pcszStr, DWORD dwStrLen, DWORD dwWidth, BOOL *pbLeadingEdge /*OUT*/, DWORD *pdwTruncWidth /*OUT*/, DWORD *pdwOffset /*OUT*/)
{
	FTFACE ftFace;

	// Verify parameters
	if (!pcszStr || !pdwOffset)
		return MGLERR_INVALIDPARAMS;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Determine the character offset
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_WidthToOffsetA(ftFace, m_font.dwStyle, pcszStr, dwStrLen,
		dwWidth, pbLeadingEdge, pdwTruncWidth, pdwOffset);

	return MGL_OK;
}

HRESULT CMglFont::WidthToOffset(const WCHAR *pcwszStr, DWORD dwStrLen, DWORD dwWidth, BOOL *pbLeadingEdge /*OUT*/, DWORD *pdwTruncWidth /*OUT*/, DWORD *pdwOffset /*OUT*/)
{
	FTFACE ftFace;

	// Verify parameters
	if (!pcwszStr || !pdwOffset)
		return MGLERR_INVALIDPARAMS;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Determine the character offset
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_WidthToOffsetW(ftFace, m_font.dwStyle, pcwszStr, dwStrLen,
		dwWidth, pbLeadingEdge, pdwTruncWidth, pdwOffset);

	return MGL_OK;
}

HRESULT CMglFont::CharsInWidth(const CHAR *pcszStr, DWORD *pdwStrWidth /*IN & OUT*/, DWORD *pdwStrLen /*IN & OUT*/, BOOL *pbFitWithinWidth /*OUT*/)
{
	FTFACE ftFace;
	
	// Verify parameters
	if (!pcszStr || !pdwStrWidth || !pdwStrLen || !pbFitWithinWidth)
		return MGLERR_INVALIDPARAMS;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Find out how many chars will fit within the width
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_CharsInWidthA(ftFace, m_font.dwStyle, pcszStr, pdwStrWidth,
		pdwStrLen, pbFitWithinWidth);
	
	return MGL_OK;
}

HRESULT CMglFont::CharsInWidth(const WCHAR *pcwszStr, DWORD *pdwStrWidth /*IN & OUT*/, DWORD *pdwStrLen /*IN & OUT*/, BOOL *pbFitWithinWidth /*OUT*/)
{
	FTFACE ftFace;
	
	// Verify parameters
	if (!pcwszStr || !pdwStrWidth || !pdwStrLen || !pbFitWithinWidth)
		return MGLERR_INVALIDPARAMS;
		
	// If there's no font face, bail out.
	if (!m_font.pFtFace) return MGLERR_NOTINITIALIZED;
	
	// ------------------------------------------------
	// Find out how many chars will fit within the width
	// ------------------------------------------------
	
	ftFace = (FTFACE)m_font.pFtFace;
	FT_CharsInWidthW(ftFace, m_font.dwStyle, pcwszStr, pdwStrWidth,
		pdwStrLen, pbFitWithinWidth);
	
	return MGL_OK;
}