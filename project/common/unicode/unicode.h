/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __UNICODE_H__
#define __UNICODE_H__

/**************************************************************
* Types
**************************************************************/

// The UNICODE_WCHAR type is defined so that
// we don't have to include our project-specific headers...
// this makes things more portable... - ptm
typedef unsigned short	UNICODE_WCHAR;	/* basically just a WCHAR type */

#ifndef NULL
#define NULL	0
#endif	// NULL

/**************************************************************
* Constants
**************************************************************/

/* Type 1 flags */
#ifndef C1_UPPER
#define C1_UPPER		0x0001
#endif

#ifndef C1_LOWER
#define C1_LOWER		0x0002
#endif

#ifndef C1_DIGIT
#define C1_DIGIT		0x0004
#endif

#ifndef C1_SPACE
#define C1_SPACE		0x0008
#endif

#ifndef C1_PUNCT
#define C1_PUNCT		0x0010
#endif

#ifndef C1_CNTRL
#define C1_CNTRL		0x0020
#endif

#ifndef C1_BLANK
#define C1_BLANK		0x0040
#endif

#ifndef C1_XDIGIT
#define C1_XDIGIT		0x0080
#endif

#ifndef C1_ALPHA
#define C1_ALPHA		0x0100
#endif

#ifndef MAX_LEADBYTES
#define MAX_LEADBYTES     12
#endif

#ifndef MAX_DEFAULTCHAR
#define MAX_DEFAULTCHAR   2
#endif

#ifndef WC_DISCARDNS
#define WC_DISCARDNS         0x0010
#endif

#ifndef WC_SEPCHARS
#define WC_SEPCHARS          0x0020
#endif

#ifndef WC_DEFAULTCHAR
#define WC_DEFAULTCHAR       0x0040
#endif

#ifndef WC_COMPOSITECHECK
#define WC_COMPOSITECHECK    0x0200
#endif

#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS 0x0400
#endif

#ifndef MAP_FOLDCZONE
#define MAP_FOLDCZONE        0x0010
#endif

#ifndef MAP_PRECOMPOSED
#define MAP_PRECOMPOSED      0x0020
#endif

#ifndef MAP_COMPOSITE
#define MAP_COMPOSITE        0x0040
#endif

#ifndef MAP_FOLDDIGITS
#define MAP_FOLDDIGITS       0x0080
#endif

#ifndef MAP_EXPAND_LIGATURES
#define MAP_EXPAND_LIGATURES 0x2000
#endif

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

/**************************************************************
* Structures
**************************************************************/

/* code page info common to SBCS and DBCS */
struct cp_info
{
    unsigned int          codepage;          /* codepage id */
    unsigned int          char_size;         /* char size (1 or 2 bytes) */
    UNICODE_WCHAR                 def_char;          /* default char value (can be double-byte) */
    UNICODE_WCHAR                 def_unicode_char;  /* default Unicode char value */
    const char           *name;              /* code page name */
};

struct sbcs_table
{
    struct cp_info        info;
    const UNICODE_WCHAR          *cp2uni;            /* code page -> Unicode map */
    const unsigned char  *uni2cp_low;        /* Unicode -> code page map */
    const unsigned short *uni2cp_high;
};

struct dbcs_table
{
    struct cp_info        info;
    const UNICODE_WCHAR          *cp2uni;            /* code page -> Unicode map */
    const unsigned char  *cp2uni_leadbytes;
    const unsigned short *uni2cp_low;        /* Unicode -> code page map */
    const unsigned short *uni2cp_high;
    unsigned char         lead_bytes[12];    /* lead bytes ranges */
};

union cptable
{
    struct cp_info    info;
    struct sbcs_table sbcs;
    struct dbcs_table dbcs;
};

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* MBTOWC Functions
**************************************************************/

/* return -1 on dst buffer overflow, -2 on invalid input char */
int cp_mbstowcs( const union cptable *table, int flags,
                      const char *s, int srclen,
                      UNICODE_WCHAR *dst, int dstlen );
                      
/* CP_SYMBOL implementation */
/* return -1 on dst buffer overflow */
int cpsymbol_mbstowcs( const char *src, int srclen, UNICODE_WCHAR *dst, int dstlen);

/**************************************************************
* WCTOMB Functions
**************************************************************/

/* wide char to multi byte string conversion */
/* return -1 on dst buffer overflow */
int cp_wcstombs( const union cptable *table, int flags,
                      const UNICODE_WCHAR *src, int srclen,
                      char *dst, int dstlen, const char *defchar, int *used );
                      
/* CP_SYMBOL implementation */
/* return -1 on dst buffer overflow, -2 on invalid character */
int cpsymbol_wcstombs( const UNICODE_WCHAR *src, int srclen, char *dst, int dstlen);

/**************************************************************
* UTF8 Functions
**************************************************************/

/* wide char to UTF-8 string conversion */
/* return -1 on dst buffer overflow */
int utf8_wcstombs( const UNICODE_WCHAR *src, int srclen, char *dst, int dstlen );

/* UTF-8 to wide char string conversion */
/* return -1 on dst buffer overflow, -2 on invalid input char */
int utf8_mbstowcs( int flags, const char *src, int srclen, UNICODE_WCHAR *dst, int dstlen );

/**************************************************************
* CodePage Functions
**************************************************************/

/* get the table of a given code page */
const union cptable *cp_get_table( unsigned int codepage );

/* enum valid codepages */
const union cptable *cp_enum_table( unsigned int index );

/**************************************************************
* String Functions
**************************************************************/

int is_dbcs_leadbyte( const union cptable *table, unsigned char ch );

UNICODE_WCHAR tolowerW( UNICODE_WCHAR ch );

UNICODE_WCHAR toupperW( UNICODE_WCHAR ch );

/* the character type contains the C1_* flags in the low 12 bits */
/* and the C2_* type in the high 4 bits */
unsigned short get_char_typeW( UNICODE_WCHAR ch );

int iscntrlW( UNICODE_WCHAR wc );

int ispunctW( UNICODE_WCHAR wc );

int isspaceW( UNICODE_WCHAR wc );

int isdigitW( UNICODE_WCHAR wc );

int isxdigitW( UNICODE_WCHAR wc );

int islowerW( UNICODE_WCHAR wc );

int isupperW( UNICODE_WCHAR wc );

int isalnumW( UNICODE_WCHAR wc );

int isalphaW( UNICODE_WCHAR wc );

int isgraphW( UNICODE_WCHAR wc );

int isprintW( UNICODE_WCHAR wc );

unsigned int strlenW( const UNICODE_WCHAR *str );

UNICODE_WCHAR *strcpyW( UNICODE_WCHAR *dst, const UNICODE_WCHAR *src );

int strcmpW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2 );

int strncmpW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2, int n );

UNICODE_WCHAR *strcatW( UNICODE_WCHAR *dst, const UNICODE_WCHAR *src );

UNICODE_WCHAR *strchrW( const UNICODE_WCHAR *str, UNICODE_WCHAR ch );

UNICODE_WCHAR *strrchrW( const UNICODE_WCHAR *str, UNICODE_WCHAR ch );

UNICODE_WCHAR *strpbrkW( const UNICODE_WCHAR *str, const UNICODE_WCHAR *accept );

unsigned long strspnW( const UNICODE_WCHAR *str, const UNICODE_WCHAR *accept );

unsigned long strcspnW( const UNICODE_WCHAR *str, const UNICODE_WCHAR *reject );

UNICODE_WCHAR *strlwrW( UNICODE_WCHAR *str );

UNICODE_WCHAR *struprW( UNICODE_WCHAR *str );

UNICODE_WCHAR *memchrW( const UNICODE_WCHAR *ptr, UNICODE_WCHAR ch, unsigned long n );

UNICODE_WCHAR *memrchrW( const UNICODE_WCHAR *ptr, UNICODE_WCHAR ch, unsigned long n );

long int atolW( const UNICODE_WCHAR *str );

int atoiW( const UNICODE_WCHAR *str );

int strcmpiW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2 );

int strncmpiW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2, int n );

int memicmpW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2, int n );

UNICODE_WCHAR *strstrW( const UNICODE_WCHAR *str, const UNICODE_WCHAR *sub );

long int strtolW( const UNICODE_WCHAR *nptr, UNICODE_WCHAR **endptr, int base );

unsigned long int strtoulW( const UNICODE_WCHAR *nptr, UNICODE_WCHAR **endptr, int base );

char* lstrcpynA(char * dst, const char * src, int n );

UNICODE_WCHAR* lstrcpynW(UNICODE_WCHAR *dst, const UNICODE_WCHAR *src, int n );

#ifdef __cplusplus
}
#endif


#endif // __UNICODE_H__