/***************************************************************************/
/*                                                                         */
/*  ftstdlib.h                                                             */
/*                                                                         */
/*    ANSI-specific library and header configuration file (specification   */
/*    only).                                                               */
/*                                                                         */
/*  Copyright 2002, 2003, 2004 by                                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This file is used to group all #includes to the ANSI C library that   */
  /* FreeType normally requires.  It also defines macros to rename the     */
  /* standard functions within the FreeType source code.                   */
  /*                                                                       */
  /* Load a file which defines __FTSTDLIB_H__ before this one to override  */
  /* it.                                                                   */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FTSTDLIB_H__
#define __FTSTDLIB_H__

/**************************************************************
* FreeType (compatibility) StdLib Functions
**************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void* ftsl_malloc(unsigned long size);
void* ftsl_realloc(void *p, unsigned long size);
void ftsl_free(void *p);

void *ftsl_memset(void *s, int c, unsigned long n);
void *ftsl_memcpy(void *t, const void *f, unsigned long s);
void *ftsl_memmove(void *dest, const void *src, unsigned long size );
int ftsl_memcmp(const void *buf1, const void *buf2, unsigned long size);

int ftsl_isalnum(int c);
int ftsl_isalpha(int c);
int ftsl_iscntrl(int c);
int ftsl_isdigit(int c);
int ftsl_isgraph(int c);
int ftsl_islower(int c);
int ftsl_isprint(int c);
int ftsl_ispunct(int c);
int ftsl_isspace(int c);
int ftsl_isupper(int c);
int ftsl_isxdigit(int c);

char* ftsl_strcat(char *ret, register const char *s2);
int ftsl_strcmp(const char *s1, const char *s2);
char* ftsl_strcpy(char* dst, const char* src);
int ftsl_strlen (const char *string);
int ftsl_strncmp(const char *s1, const char *s2, int n);
char *ftsl_strncpy(char *ret, const char *s2, int n);
char *ftsl_strchr(const char *string, int chr);
char *ftsl_strrchr(const char *string, int chr);
char *ftsl_strdup(char *string);

void ftsl_sprintf(char *text, char *s, ...);
int ftsl_atoi(const char *s);
long ftsl_atol(const char *s);
long ftsl_labs(long a);
void ftsl_exit(int i);

void ftsl_qsort(void *basep, unsigned long nelems, unsigned long size, int (*comp)(const void *, const void *));

#ifdef __cplusplus
}
#endif

/**************************************************************/

//#include <stddef.h>

// Defines the number of bits in a CHAR type
#define FT_CHAR_BIT		8

typedef int _ft_ptrdiff_t;
#define ft_ptrdiff_t  	_ft_ptrdiff_t

  /**********************************************************************/
  /*                                                                    */
  /*                           integer limits                           */
  /*                                                                    */
  /* UINT_MAX and ULONG_MAX are used to automatically compute the size  */
  /* of `int' and `long' in bytes at compile-time.  So far, this works  */
  /* for all platforms the library has been tested on.                  */
  /*                                                                    */
  /* Note that on the extremely rare platforms that do not provide      */
  /* integer types that are _exactly_ 16 and 32 bits wide (e.g. some    */
  /* old Crays where `int' is 36 bits), we do not make any guarantee    */
  /* about the correct behaviour of FT2 with all fonts.                 */
  /*                                                                    */
  /* In these case, "ftconfig.h" will refuse to compile anyway with a   */
  /* message like "couldn't find 32-bit type" or something similar.     */
  /*                                                                    */
  /* IMPORTANT NOTE: We do not define aliases for heap management and   */
  /*                 i/o routines (i.e. malloc/free/fopen/fread/...)    */
  /*                 since these functions should all be encapsulated   */
  /*                 by platform-specific implementations of            */
  /*                 "ftsystem.c".                                      */
  /*                                                                    */
  /**********************************************************************/


//#include <limits.h>

#define FT_UINT_MAX   		0xffffffffU
#define FT_INT_MAX    		0x7fffffff
#define FT_ULONG_MAX  	0xffffffffUL


  /**********************************************************************/
  /*                                                                    */
  /*                 character and string processing                    */
  /*                                                                    */
  /**********************************************************************/


//#include <ctype.h>

#define ft_isalnum   ftsl_isalnum
#define ft_isupper   ftsl_isupper
#define ft_islower   ftsl_islower
#define ft_isdigit   ftsl_isdigit
#define ft_isxdigit  ftsl_isxdigit


//#include <string.h>

#define ft_memcmp   ftsl_memcmp
#define ft_memcpy   ftsl_memcpy
#define ft_memmove  ftsl_memmove
#define ft_memset   ftsl_memset
#define ft_strcat   ftsl_strcat
#define ft_strcmp   ftsl_strcmp
#define ft_strcpy   ftsl_strcpy
#define ft_strlen   ftsl_strlen
#define ft_strncmp  ftsl_strncmp
#define ft_strncpy  ftsl_strncpy
#define ft_strrchr  ftsl_strrchr


//#include <stdio.h>

#define ft_sprintf  ftsl_sprintf


  /**********************************************************************/
  /*                                                                    */
  /*                             sorting                                */
  /*                                                                    */
  /**********************************************************************/


#define ft_qsort  ftsl_qsort
#define ft_exit   ftsl_exit    /* only used to exit from unhandled exceptions */

#define ft_atol   ftsl_atol


  /**********************************************************************/
  /*                                                                    */
  /*                         execution control                          */
  /*                                                                    */
  /**********************************************************************/

#ifdef _WIN32
// Use the x86-native setjmp for PalmSim
#include <setjmp.h>

#define ft_jmp_buf    jmp_buf /* note: this cannot be a typedef since */
                              /*       jmp_buf is defined as a macro  */
                              /*       on certain platforms           */

#define ft_setjmp   setjmp    /* same thing here */
#define ft_longjmp  longjmp   /* "               */

#else // if defined(__PALMOS_ARMLET__)

#include "../../../../setjmp_arm/setjmp_arm.h"

#define ft_jmp_buf    jmp_buf_arm /* note: this cannot be a typedef since */
                              /*       jmp_buf is defined as a macro  */
                              /*       on certain platforms           */

#define ft_setjmp   setjmp_arm    /* same thing here */
#define ft_longjmp  longjmp_arm   /* "               */

#endif

  /* the following is only used for debugging purposes, i.e. when */
  /* FT_DEBUG_LEVEL_ERROR or FT_DEBUG_LEVEL_TRACE are defined     */
  /*                                                              */
//#include <stdarg.h>


#endif /* __FTSTDLIB_H__ */


/* END */
