/*
 * Unicode string manipulation functions
 *
 * Copyright 2000 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "unicode.h"

#ifndef MB_LEN_MAX
#define MB_LEN_MAX  2        /* max. # bytes in multibyte char */
#define SHRT_MIN    (-32768)    /* minimum (signed) short value */
#define SHRT_MAX    32767     /* maximum (signed) short value */
#define USHRT_MAX   0xffff    /* maximum unsigned short value */
#define INT_MIN     (-2147483647 - 1) /* minimum (signed) int value */
#define INT_MAX     2147483647    /* maximum (signed) int value */
#define UINT_MAX    0xffffffff    /* maximum unsigned int value */
#define LONG_MIN    (-2147483647L - 1) /* minimum (signed) long value */
#define LONG_MAX    2147483647L    /* maximum (signed) long value */
#define ULONG_MAX   0xffffffffUL    /* maximum unsigned long value */
#endif

extern const UNICODE_WCHAR casemap_lower[];
extern const UNICODE_WCHAR casemap_upper[];
extern const unsigned short wctype_table[];

int is_dbcs_leadbyte( const union cptable *table, unsigned char ch )
{
    return (table->info.char_size == 2) && (table->dbcs.cp2uni_leadbytes[ch]);
}

UNICODE_WCHAR tolowerW( UNICODE_WCHAR ch )
{
    return ch + casemap_lower[casemap_lower[ch >> 8] + (ch & 0xff)];
}

UNICODE_WCHAR toupperW( UNICODE_WCHAR ch )
{
    return ch + casemap_upper[casemap_upper[ch >> 8] + (ch & 0xff)];
}

/* the character type contains the C1_* flags in the low 12 bits */
/* and the C2_* type in the high 4 bits */
unsigned short get_char_typeW( UNICODE_WCHAR ch )
{
    return wctype_table[wctype_table[ch >> 8] + (ch & 0xff)];
}

int iscntrlW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & C1_CNTRL;
}

int ispunctW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & C1_PUNCT;
}

int isspaceW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & C1_SPACE;
}

int isdigitW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & C1_DIGIT;
}

int isxdigitW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & C1_XDIGIT;
}

int islowerW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & C1_LOWER;
}

int isupperW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & C1_UPPER;
}

int isalnumW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & (C1_ALPHA|C1_DIGIT|C1_LOWER|C1_UPPER);
}

int isalphaW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & (C1_ALPHA|C1_LOWER|C1_UPPER);
}

int isgraphW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & (C1_ALPHA|C1_PUNCT|C1_DIGIT|C1_LOWER|C1_UPPER);
}

int isprintW( UNICODE_WCHAR wc )
{
    return get_char_typeW(wc) & (C1_ALPHA|C1_BLANK|C1_PUNCT|C1_DIGIT|C1_LOWER|C1_UPPER);
}

unsigned int strlenW( const UNICODE_WCHAR *str )
{
    const UNICODE_WCHAR *s = str;
    while (*s) s++;
    return s - str;
}

UNICODE_WCHAR *strcpyW( UNICODE_WCHAR *dst, const UNICODE_WCHAR *src )
{
    UNICODE_WCHAR *p = dst;
    while ((*p++ = *src++))
    	;
    return dst;
}

int strcmpW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2 )
{
    while (*str1 && (*str1 == *str2)) { str1++; str2++; }
    return *str1 - *str2;
}

int strncmpW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2, int n )
{
    if (n <= 0) return 0;
    while ((--n > 0) && *str1 && (*str1 == *str2)) { str1++; str2++; }
    return *str1 - *str2;
}

UNICODE_WCHAR *strcatW( UNICODE_WCHAR *dst, const UNICODE_WCHAR *src )
{
    strcpyW( dst + strlenW(dst), src );
    return dst;
}

UNICODE_WCHAR *strchrW( const UNICODE_WCHAR *str, UNICODE_WCHAR ch )
{
    do { if (*str == ch) return (UNICODE_WCHAR *)str; } while (*str++);
    return NULL;
}

UNICODE_WCHAR *strrchrW( const UNICODE_WCHAR *str, UNICODE_WCHAR ch )
{
    UNICODE_WCHAR *ret = NULL;
    do { if (*str == ch) ret = (UNICODE_WCHAR *)str; } while (*str++);
    return ret;
}

UNICODE_WCHAR *strpbrkW( const UNICODE_WCHAR *str, const UNICODE_WCHAR *accept )
{
    for ( ; *str; str++) if (strchrW( accept, *str )) return (UNICODE_WCHAR *)str;
    return NULL;
}

unsigned long strspnW( const UNICODE_WCHAR *str, const UNICODE_WCHAR *accept )
{
    const UNICODE_WCHAR *ptr;
    for (ptr = str; *ptr; ptr++) if (!strchrW( accept, *ptr )) break;
    return ptr - str;
}

unsigned long strcspnW( const UNICODE_WCHAR *str, const UNICODE_WCHAR *reject )
{
    const UNICODE_WCHAR *ptr;
    for (ptr = str; *ptr; ptr++) if (strchrW( reject, *ptr )) break;
    return ptr - str;
}

UNICODE_WCHAR *strlwrW( UNICODE_WCHAR *str )
{
    UNICODE_WCHAR *ret = str;
    while ((*str = tolowerW(*str))) str++;
    return ret;
}

UNICODE_WCHAR *struprW( UNICODE_WCHAR *str )
{
    UNICODE_WCHAR *ret = str;
    while ((*str = toupperW(*str))) str++;
    return ret;
}

UNICODE_WCHAR *memchrW( const UNICODE_WCHAR *ptr, UNICODE_WCHAR ch, unsigned long n )
{
    const UNICODE_WCHAR *end;
    for (end = ptr + n; ptr < end; ptr++) if (*ptr == ch) return (UNICODE_WCHAR *)ptr;
    return NULL;
}

UNICODE_WCHAR *memrchrW( const UNICODE_WCHAR *ptr, UNICODE_WCHAR ch, unsigned long n )
{
    const UNICODE_WCHAR *end, *ret = NULL;
    for (end = ptr + n; ptr < end; ptr++) if (*ptr == ch) ret = ptr;
    return (UNICODE_WCHAR *)ret;
}

long int atolW( const UNICODE_WCHAR *str )
{
    return strtolW( str, (UNICODE_WCHAR **)0, 10 );
}

int atoiW( const UNICODE_WCHAR *str )
{
    return (int)atolW( str );
}

int strcmpiW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2 )
{
    for (;;)
    {
        int ret = tolowerW(*str1) - tolowerW(*str2);
        if (ret || !*str1) return ret;
        str1++;
        str2++;
    }
}

int strncmpiW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2, int n )
{
    int ret = 0;
    for ( ; n > 0; n--, str1++, str2++)
        if ((ret = tolowerW(*str1) - tolowerW(*str2)) || !*str1) break;
    return ret;
}

int memicmpW( const UNICODE_WCHAR *str1, const UNICODE_WCHAR *str2, int n )
{
    int ret = 0;
    for ( ; n > 0; n--, str1++, str2++)
        if ((ret = tolowerW(*str1) - tolowerW(*str2))) break;
    return ret;
}

UNICODE_WCHAR *strstrW( const UNICODE_WCHAR *str, const UNICODE_WCHAR *sub )
{
    while (*str)
    {
        const UNICODE_WCHAR *p1 = str, *p2 = sub;
        while (*p1 && *p2 && *p1 == *p2) { p1++; p2++; }
        if (!*p2) return (UNICODE_WCHAR *)str;
        str++;
    }
    return NULL;
}

/* strtolW and strtoulW implementation based on the GNU C library code */
/* Copyright (C) 1991,92,94,95,96,97,98,99,2000,2001 Free Software Foundation, Inc. */

long int strtolW( const UNICODE_WCHAR *nptr, UNICODE_WCHAR **endptr, int base )
{
  int negative;
  register unsigned long int cutoff;
  register unsigned int cutlim;
  register unsigned long int i;
  register const UNICODE_WCHAR *s;
  register UNICODE_WCHAR c;
  const UNICODE_WCHAR *save, *end;
  int overflow;

  if (base < 0 || base == 1 || base > 36) return 0;

  save = s = nptr;

  /* Skip white space.  */
  while (isspaceW (*s))
    ++s;
  if (!*s) goto noconv;

  /* Check for a sign.  */
  negative = 0;
  if (*s == '-')
    {
      negative = 1;
      ++s;
    }
  else if (*s == '+')
    ++s;

  /* Recognize number prefix and if BASE is zero, figure it out ourselves.  */
  if (*s == '0')
    {
      if ((base == 0 || base == 16) && toupperW(s[1]) == 'X')
	{
	  s += 2;
	  base = 16;
	}
      else if (base == 0)
	base = 8;
    }
  else if (base == 0)
    base = 10;

  /* Save the pointer so we can check later if anything happened.  */
  save = s;
  end = NULL;

  cutoff = ULONG_MAX / (unsigned long int) base;
  cutlim = ULONG_MAX % (unsigned long int) base;

  overflow = 0;
  i = 0;
  c = *s;
  for (;c != '\0'; c = *++s)
  {
      if (s == end)
          break;
      if (c >= '0' && c <= '9')
          c -= '0';
      else if (isalphaW (c))
          c = toupperW (c) - 'A' + 10;
      else
          break;
      if ((int) c >= base)
          break;
      /* Check for overflow.  */
      if (i > cutoff || (i == cutoff && c > cutlim))
          overflow = 1;
      else
      {
          i *= (unsigned long int) base;
          i += c;
      }
  }

  /* Check if anything actually happened.  */
  if (s == save)
    goto noconv;

  /* Store in ENDPTR the address of one character
     past the last character we converted.  */
  if (endptr != NULL)
    *endptr = (UNICODE_WCHAR *)s;

  /* Check for a value that is within the range of
     `unsigned LONG int', but outside the range of `LONG int'.  */
  if (overflow == 0
      && i > (negative
	      ? -((unsigned long int) (LONG_MIN + 1)) + 1
	      : (unsigned long int) LONG_MAX))
    overflow = 1;

  if (overflow)
    {
      return negative ? LONG_MIN : LONG_MAX;
    }

  /* Return the result of the appropriate sign.  */
  return negative ? -i : i;

noconv:
  /* We must handle a special case here: the base is 0 or 16 and the
     first two characters are '0' and 'x', but the rest are not
     hexadecimal digits.  This is no error case.  We return 0 and
     ENDPTR points to the `x`.  */
  if (endptr != NULL)
    {
      if (save - nptr >= 2 && toupperW (save[-1]) == 'X'
	  && save[-2] == '0')
	*endptr = (UNICODE_WCHAR *)&save[-1];
      else
	/*  There was no number to convert.  */
	*endptr = (UNICODE_WCHAR *)nptr;
    }

  return 0L;
}


unsigned long int strtoulW( const UNICODE_WCHAR *nptr, UNICODE_WCHAR **endptr, int base )
{
  int negative;
  register unsigned long int cutoff;
  register unsigned int cutlim;
  register unsigned long int i;
  register const UNICODE_WCHAR *s;
  register UNICODE_WCHAR c;
  const UNICODE_WCHAR *save, *end;
  int overflow;

  if (base < 0 || base == 1 || base > 36) return 0;

  save = s = nptr;

  /* Skip white space.  */
  while (isspaceW (*s))
    ++s;
  if (!*s) goto noconv;

  /* Check for a sign.  */
  negative = 0;
  if (*s == '-')
    {
      negative = 1;
      ++s;
    }
  else if (*s == '+')
    ++s;

  /* Recognize number prefix and if BASE is zero, figure it out ourselves.  */
  if (*s == '0')
    {
      if ((base == 0 || base == 16) && toupperW(s[1]) == 'X')
	{
	  s += 2;
	  base = 16;
	}
      else if (base == 0)
	base = 8;
    }
  else if (base == 0)
    base = 10;

  /* Save the pointer so we can check later if anything happened.  */
  save = s;
  end = NULL;

  cutoff = ULONG_MAX / (unsigned long int) base;
  cutlim = ULONG_MAX % (unsigned long int) base;

  overflow = 0;
  i = 0;
  c = *s;
  for (;c != '\0'; c = *++s)
  {
      if (s == end)
          break;
      if (c >= '0' && c <= '9')
          c -= '0';
      else if (isalphaW (c))
          c = toupperW (c) - 'A' + 10;
      else
          break;
      if ((int) c >= base)
          break;
      /* Check for overflow.  */
      if (i > cutoff || (i == cutoff && c > cutlim))
          overflow = 1;
      else
      {
          i *= (unsigned long int) base;
          i += c;
      }
  }

  /* Check if anything actually happened.  */
  if (s == save)
    goto noconv;

  /* Store in ENDPTR the address of one character
     past the last character we converted.  */
  if (endptr != NULL)
    *endptr = (UNICODE_WCHAR *)s;

  if (overflow)
    {
      return ULONG_MAX;
    }

  /* Return the result of the appropriate sign.  */
  return negative ? -i : i;

noconv:
  /* We must handle a special case here: the base is 0 or 16 and the
     first two characters are '0' and 'x', but the rest are not
     hexadecimal digits.  This is no error case.  We return 0 and
     ENDPTR points to the `x`.  */
  if (endptr != NULL)
    {
      if (save - nptr >= 2 && toupperW (save[-1]) == 'X'
	  && save[-2] == '0')
	*endptr = (UNICODE_WCHAR *)&save[-1];
      else
	/*  There was no number to convert.  */
	*endptr = (UNICODE_WCHAR *)nptr;
    }

  return 0L;
}

/***********************************************************************
 *           lstrcpynA
 *           lstrcpyn
 *
 * Note: this function differs from the UNIX strncpy, it _always_ writes
 * a terminating \0.
 *
 * Note: n is an INT but Windows treats it as unsigned, and will happily
 * copy a gazillion chars if n is negative.
 */
char* lstrcpynA(char * dst, const char * src, int n )
{
    char* d;
    const char* s;
    unsigned long count;

     if (!dst || !src)
     	return dst;
     	
     d = dst;
     s = src;
     count = n;

        while ((count > 1) && *s)
        {
            count--;
            *d++ = *s++;
        }
        if (count) *d = 0;

    return dst;
}


/***********************************************************************
 *           lstrcpynW
 *
 * Note: this function differs from the UNIX strncpy, it _always_ writes
 * a terminating \0
 *
 * Note: n is an INT but Windows treats it as unsigned, and will happily
 * copy a gazillion chars if n is negative.
 */
UNICODE_WCHAR* lstrcpynW(UNICODE_WCHAR *dst, const UNICODE_WCHAR *src, int n )
{
   UNICODE_WCHAR* d;
   const UNICODE_WCHAR* s;
   unsigned long count;

    if (!dst || !src)
    	return dst;
    	
    d = dst;
    s = src;
    count = n;

        while ((count > 1) && *s)
        {
            count--;
            *d++ = *s++;
        }
        
        if (count) *d = 0;
   
    return dst;
}
