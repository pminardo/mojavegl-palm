/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __S_STRING_H_
#define __S_STRING_H_

/**************************************************************
* String Constants
**************************************************************/

#ifndef MB_PRECOMPOSED
#define MB_PRECOMPOSED              0x01
#endif

#ifndef MB_COMPOSITE
#define MB_COMPOSITE                0x02
#endif

#ifndef MB_USEGLYPHCHARS
#define MB_USEGLYPHCHARS            0x04
#endif

#ifndef MB_ERR_INVALID_CHARS
#define MB_ERR_INVALID_CHARS        0x08
#endif

#ifndef CP_ACP
#define CP_ACP        0
#endif

#ifndef CP_OEMCP
#define CP_OEMCP      1
#endif

#ifndef CP_MACCP
#define CP_MACCP      2
#endif

#ifndef CP_THREAD_ACP
#define CP_THREAD_ACP 3
#endif

#ifndef CP_SYMBOL
#define CP_SYMBOL     42
#endif

#ifndef CP_UTF7
#define CP_UTF7       65000
#endif

#ifndef CP_UTF8
#define CP_UTF8       65001
#endif

/**************************************************************
* String Functions
**************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Initialize codepage globals for use
void S_InitCodePages(void);

// Determine if a character is a lead byte.
BOOL S_IsDBCSLeadByte( BYTE testchar );

CHAR* S_CharNextA(const CHAR *ptr);
WCHAR* S_CharNextW(const WCHAR *x);

CHAR* S_CharPrevA( const CHAR* start, const CHAR* ptr );
WCHAR* S_CharPrevW(const WCHAR* start, const WCHAR* x);

DWORD S_StrLenA(const CHAR *pcszStr);
DWORD S_StrLenW(const WCHAR *pcwszStr);

LONG S_MultiByteToWideChar(DWORD page, DWORD flags, const CHAR *src, LONG srclen,
	WCHAR *dst, LONG dstlen );
	
LONG S_WideCharToMultiByte( DWORD page, DWORD flags, const WCHAR *src, LONG srclen,
                                CHAR *dst, LONG dstlen, const CHAR *defchar, BOOL *used );

#ifdef __cplusplus
}
#endif

#endif // __S_STRING_H_