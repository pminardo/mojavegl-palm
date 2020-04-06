/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"

// Needed for FreeType stdlib
#include <ft2build.h>
#include <freetype/freetype.h>

#include "unicode.h"
#include "S_string.h"

/**************************************************************
* Globals
**************************************************************/

/* Current Code Pages */
static const union cptable *ansi_cptable = NULL;

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ String Functions --------------
#endif

void S_InitCodePages(void)
{
	// Use windows-1252 codepage for ANSI
	ansi_cptable = cp_get_table(1252);
}

/***********************************************************************
 *		get_codepage_table
 *
 * Find the table for a given codepage, handling CP_ACP etc. pseudo-codepages
 */
static const union cptable *get_codepage_table( unsigned int codepage )
{
    const union cptable *ret = NULL;

  //  assert( ansi_cptable );  /* init must have been done already */

    switch(codepage)
    {
    case CP_ACP:
        return ansi_cptable;
        
    // Neither OEMCP nor MACCP are supported (or needed) by MojaveGL
    case CP_OEMCP:
       break; //return oem_cptable;
    case CP_MACCP:
      break; // return mac_cptable;
      
    case CP_UTF7:
    case CP_UTF8:
        break;
    case CP_THREAD_ACP:
        /*if (!(codepage = kernel_get_thread_data()->code_page))*/ return ansi_cptable;
        /* fall through */
    default:
        if (codepage == ansi_cptable->info.codepage) return ansi_cptable;
       /* if (codepage == oem_cptable->info.codepage) return oem_cptable;
        if (codepage == mac_cptable->info.codepage) return mac_cptable;*/
        ret = cp_get_table( codepage );
        break;
    }
    return ret;
}

/***********************************************************************
 *           S_IsDBCSLeadByte
 *
 * Determine if a character is a lead byte.
 *
 * PARAMS
 *  testchar [I] Character to test
 *
 * RETURNS
 *  TRUE, if testchar is a lead byte in the Ansii code page,
 *  FALSE otherwise.
 */
BOOL S_IsDBCSLeadByte( BYTE testchar )
{
    if (!ansi_cptable) return FALSE;
    return is_dbcs_leadbyte( ansi_cptable, testchar );
}

CHAR* S_CharNextA(const CHAR *ptr)
{
	if (!*ptr) return (CHAR*)ptr;
	if (S_IsDBCSLeadByte( ptr[0] ) && ptr[1]) return (CHAR*)(ptr + 2);
	
	return (CHAR*)(ptr + 1);
}

WCHAR* S_CharNextW(const WCHAR *x)
{
    if (*x) x++;

    return (WCHAR*)x;
}

CHAR* S_CharPrevA( const CHAR* start, const CHAR* ptr )
{
    while (*start && (start < ptr))
    {
        const CHAR* next = S_CharNextA( start );
        if (next >= ptr) break;
        start = next;
    }
    return (CHAR*)start;
}

WCHAR* S_CharPrevW(const WCHAR* start, const WCHAR* x)
{
    if (x>start) return (WCHAR*)(x-1);
    else return (WCHAR*)x;
}

DWORD S_StrLenA(const CHAR *pcszStr)
{
	return ftsl_strlen(pcszStr);
}

DWORD S_StrLenW(const WCHAR *pcwszStr)
{
	return strlenW(pcwszStr);
}

/***********************************************************************
 *              S_MultiByteToWideChar
 *
 * Convert a multibyte character string into a Unicode string.
 *
 * PARAMS
 *   page   [I] Codepage character set to convert from
 *   flags  [I] Character mapping flags
 *   src    [I] Source string buffer
 *   srclen [I] Length of src, or -1 if src is NUL terminated
 *   dst    [O] Destination buffer
 *   dstlen [I] Length of dst, or 0 to compute the required length
 *
 * RETURNS
 *   Success: If dstlen > 0, the number of characters written to dst.
 *            If dstlen == 0, the number of characters needed to perform the
 *            conversion. In both cases the count includes the terminating NUL.
 *   Failure: 0. Use GetLastError() to determine the cause. Possible errors are
 *            ERROR_INSUFFICIENT_BUFFER, if not enough space is available in dst
 *            and dstlen != 0; ERROR_INVALID_PARAMETER,  if an invalid parameter
 *            is passed, and ERROR_NO_UNICODE_TRANSLATION if no translation is
 *            possible for src.
 */
LONG S_MultiByteToWideChar(DWORD page, DWORD flags, const CHAR *src, LONG srclen,
	WCHAR *dst, LONG dstlen )
{
    const union cptable *table;
    int ret;

    if (!src || (!dst && dstlen))
    {
        //SetLastError( ERROR_INVALID_PARAMETER );
        return 0;
    }

    if (flags & MB_USEGLYPHCHARS)
    	return 0;
   // FIXME("MB_USEGLYPHCHARS not supported\n");

    if (srclen < 0) srclen = ftsl_strlen(src) + 1;

    switch(page)
    {
    case CP_SYMBOL:
        if( flags)
        {
            //SetLastError( ERROR_INVALID_PARAMETER );
            return 0;
        }
        ret = cpsymbol_mbstowcs( src, srclen, dst, dstlen );
        break;
    case CP_UTF7:
        //FIXME("UTF-7 not supported\n");
        //SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return 0;

        /* fall through */
    case CP_UTF8:
        ret = utf8_mbstowcs( flags, src, srclen, dst, dstlen );
        break;
    default:
        if (!(table = get_codepage_table( page )))
        {
         //   SetLastError( ERROR_INVALID_PARAMETER );
            return 0;
        }
        ret = cp_mbstowcs( table, flags, src, srclen, dst, dstlen );
        break;
    } 

    if (ret < 0)
    {
       /* switch(ret)
        {
        case -1: SetLastError( ERROR_INSUFFICIENT_BUFFER ); break;
        case -2: SetLastError( ERROR_NO_UNICODE_TRANSLATION ); break;
        }*/
        ret = 0;
    }
    return ret;
}

/***********************************************************************
 *              S_WideCharToMultiByte
 *
 * Convert a Unicode character string into a multibyte string.
 *
 * PARAMS
 *   page    [I] Code page character set to convert to
 *   flags   [I] Mapping Flags (MB_ constants from "winnls.h").
 *   src     [I] Source string buffer
 *   srclen  [I] Length of src, or -1 if src is NUL terminated
 *   dst     [O] Destination buffer
 *   dstlen  [I] Length of dst, or 0 to compute the required length
 *   defchar [I] Default character to use for conversion if no exact
 *		    conversion can be made
 *   used    [O] Set if default character was used in the conversion
 *
 * RETURNS
 *   Success: If dstlen > 0, the number of characters written to dst.
 *            If dstlen == 0, number of characters needed to perform the
 *            conversion. In both cases the count includes the terminating NUL.
 *   Failure: 0. Use GetLastError() to determine the cause. Possible errors are
 *            ERROR_INSUFFICIENT_BUFFER, if not enough space is available in dst
 *            and dstlen != 0, and ERROR_INVALID_PARAMETER, if an invalid
 *            parameter was given.
 */
LONG S_WideCharToMultiByte( DWORD page, DWORD flags, const WCHAR *src, LONG srclen,
                                CHAR *dst, LONG dstlen, const CHAR *defchar, BOOL *used )
{
    const union cptable *table;
    int ret, used_tmp;

    if (!src || (!dst && dstlen))
    {
        //SetLastError( ERROR_INVALID_PARAMETER );
        return 0;
    }

    if (srclen < 0) srclen = strlenW(src) + 1;

    switch(page)
    {
    case CP_SYMBOL:
        if( flags || defchar || used)
        {
            //SetLastError( ERROR_INVALID_PARAMETER );
            return 0;
        }
        ret = cpsymbol_wcstombs( src, srclen, dst, dstlen );
        break;
    case CP_UTF7:
        //FIXME("UTF-7 not supported\n");
        //SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return 0;

        /* fall through */
    case CP_UTF8:
        if (used) *used = FALSE;  /* all chars are valid for UTF-8 */
        ret = utf8_wcstombs( src, srclen, dst, dstlen );
        break;
    default:
        if (!(table = get_codepage_table( page )))
        {
            //SetLastError( ERROR_INVALID_PARAMETER );
            return 0;
        }
        ret = cp_wcstombs( table, flags, src, srclen, dst, dstlen,
                                defchar, used ? &used_tmp : NULL );
        if (used) *used = used_tmp;
        break;
    }

    if (ret < 0)
    {
        /*switch(ret)
        {
        case -1: SetLastError( ERROR_INSUFFICIENT_BUFFER ); break;
        case -2: SetLastError( ERROR_NO_UNICODE_TRANSLATION ); break;
        }*/
        ret = 0;
    }
    //TRACE("cp %d %s -> %s\n", page, debugstr_w(src), debugstr_a(dst));
    return ret;
}