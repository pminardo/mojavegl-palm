/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* FreeType (compatibility) StdLib
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/config/ftconfig.h>

/**************************************************************
* Macros
**************************************************************/

#ifndef min
#define min(a,b)							(((a)<(b))?(a):(b)) 
#endif

/**************************************************************
* Internal Functions
**************************************************************/

void* ftsl_malloc(unsigned long size)
{
	return Malloc(size);
}

void* ftsl_realloc(void *p, unsigned long size)
{
	return Realloc(p, size);
}

void ftsl_free(void *p)
{
	Free(p);
}

void *ftsl_memset(void *s, int c, unsigned long n)
{
	MemSet(s, n, c);
	
	return s;
}

void *ftsl_memcpy(void *t, const void *f, unsigned long s)
{
	MemMove(t, f, s);
	
	return t;
}

void *ftsl_memmove(void *dest, const void *src, unsigned long size )
{
	MemMove(dest,src,size);

	return dest;
}
 
int ftsl_memcmp(const void *buf1, const void *buf2, unsigned long size)
{
	return (int)MemCmp(buf1, buf2, size);
}

int ftsl_isalnum(int c)
{
	if ((ftsl_isalpha(c)) || (ftsl_isdigit(c)))
		return 1;
	return 0;
}

int ftsl_isalpha(int c)
{
	if ((ftsl_isupper(c)) || (ftsl_islower(c)))
	  return 1;
	return 0;
}

/* Not sure which characters are actually control characters in Cybikoland */
int ftsl_iscntrl(int c)
{
	if ((c >= 0) && (c <= 31))
		return 1;
	return 0;
}

int ftsl_isdigit(int c)
{
	if ((c >= 48) && (c <= 57))
		return 1;
	return 0;
}

/*  Not quite sure this one is correct either */
int ftsl_isgraph(int c)
{
	if ((c >= 33) && (c <= 255))
	  return 1;
	return 0;
}

int ftsl_islower(int c)
{
	if ((c >= 97) && (c <= 122))
		return 1;
	return 0;
}

/*  Not quite sure this one is correct either */
int ftsl_isprint(int c)
{
	if ((c >= 32) && (c <= 255))
	  return 1;
	return 0;
}


int ftsl_ispunct(int c)
{
	if ((!ftsl_isalnum(c)) && (!ftsl_isspace(c)) && (ftsl_isprint(c)))
		return 1;
	return 0;
}

int ftsl_isspace(int c)
{
	if (c == 32)
		return 1;
	return 0;
}

int ftsl_isupper(int c)
{
	if ((c >= 65) && (c <= 90))
		return 1;
	return 0;
}

/* Returns a nonzero value if c is a hex digit (0-9,a-f,A-F), zero otherwise */
int ftsl_isxdigit(int c)
{
	if (((c >= 48) && (c<=57)) || ((c >= 65) && (c <= 70)) || ((c >= 97) && (c <= 102)))
		return 1;
	return 0;
}

char* ftsl_strcat(char *ret, register const char *s2)
{
	char *s1 = ret;

	while (*s1++ != '\0')
		/* EMPTY */ ;
	s1--;
	while ((*s1++ = *s2++))
		/* EMPTY */ ;
	return ret;
}

int ftsl_strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++) {
		if (*s1++ == '\0') {
			return 0;
		}
	}
	if (*s1 == '\0') return -1;
	if (*--s2 == '\0') return 1;
	return (unsigned char) *s1 - (unsigned char) *s2;
}

char* ftsl_strcpy(char* dst, const char* src)
{
	char*	tmp = dst;
		
		if (!dst || !src)
			return dst;

	while (*src)
	{
		*tmp++ = *src++;
	}
	
	*tmp = 0;
	return(dst);
}

int ftsl_strlen (const char *string)
{
	const char *s = string;

	while (*s++)
		/* EMPTY */ ;

	return --s - string;
}

int ftsl_strncmp(const char *s1, const char *s2, int n)
{
  unsigned char u1, u2;

  while (n-- > 0)
    {
      u1 = (unsigned char) *s1++;
      u2 = (unsigned char) *s2++;
      if (u1 != u2)
	return u1 - u2;
      if (u1 == '\0')
	return 0;
    }
  return 0;
}

char *ftsl_strncpy(char *ret, const char *s2, int n)
{
	char *s1 = ret;

	if (n>0) {
		while((*s1++ = *s2++) && --n > 0)
			/* EMPTY */ ;
		if ((*--s2 == '\0') && --n > 0) {
			do {
				*s1++ = '\0';
			} while(--n > 0);
		}
	}
	return ret;
}

char *ftsl_strchr(const char *string, int chr)
{
	chr = (char) chr;

	while (chr != *string) {
		if (*string++ == '\0') return (char*)0;
	}
	return (char *)string;
}

char *ftsl_strrchr(const char *string, int chr)
{
	const char *ptr = string;

	while ((string = ftsl_strchr(string, chr)) != NULL)
	{
		string++;
		ptr = string;
	}
	
	return (char*)ptr;
}

char *ftsl_strdup(char *string)
{
	return Strdup(string);
}


#define write_unaligned8( p, v ) *(p) = v

static void write_unaligned32( unsigned char* dest, unsigned long val )
{
    int i;
    dest += sizeof(val);
    for ( i = 0; i < sizeof(val); ++i ) {
        *--dest = val & 0x000000FF;
        val >>= 8;
    }
} /* write_unaligned32 */

static void write_unaligned16( unsigned char* dest, unsigned short val )
{
    int i;

    dest += sizeof(val);
    for ( i = 0; i < sizeof(val); ++i ) {
        *--dest = val & 0x00FF;
        val >>= 8;
    }
} /* write_unaligned16 */

#define read_unaligned8(cp) (*(cp))

static unsigned short read_unaligned16( const unsigned char* src )
{
    int i;
    unsigned short val = 0;

    for ( i = 0; i < sizeof(val); ++i ) {
        val <<= 8;
        val |= *src++;
    }

    return val;
} /* read_unaligned16 */

static unsigned long read_unaligned32( const unsigned char* src )
{
    int i;
    unsigned long val = 0;

    for ( i = 0; i < sizeof(val); ++i ) {
        val <<= 8;
        val |= *src++;
    }

    return val;
} /* read_unaligned32 */

#define STACK_START(typ, var, siz ) typ var[siz]
#define STACK_END(s)
#define ADD_TO_STACK4(s,val,offset) \
    write_unaligned32( &s[offset], (unsigned long)val )
#define ADD_TO_STACK2(s,val,offset) \
    write_unaligned16( &s[offset], (unsigned short)val )
#define ADD_TO_STACK1(s,val,offset) \
    s[offset] = (unsigned char)val; \
    s[offset+1] = 0

/* from file StringMgr.h */
static Int16 ftsl_StrVPrintF( Char* s, const Char* formatStr, _Palm_va_list arg )
{
    Int16 result;
    unsigned long* argv_arm = (unsigned long*)arg;
    unsigned char argv_68k[48];
    unsigned short done, isLong, innerDone, useArg;
    unsigned char* str = (unsigned char*)formatStr;
    unsigned short offset = 0;

    for ( done = 0; !done; ) {
        switch( *str++ ) {
        case '\0':
            done = 1; 
            break;
        case '%':
            isLong = useArg = 0;
            for( innerDone = 0; !innerDone; ) {
                unsigned char nxt = *str++;
                switch( nxt ) {                
                case '%':
                    innerDone = 1;
                    break;
                case 'l':
                    isLong = 1;
                    break;
                case 's':
                    isLong = 1;
                case 'd':
                case 'x':
                case 'c':
                    innerDone = 1;
                    useArg = 1;
                    break;
                case 'g':
                case 'f':
                    innerDone = 1;
                    break;
                default:
                    if ( nxt >= '0' && nxt <= '9' ) {
                        /* accept %4x */
                    } else {
                    }
                }
            }

            if ( useArg ) {
                unsigned long param;
                param = *argv_arm++;
                if ( isLong ) {
                    write_unaligned32( &argv_68k[offset], param );
                    offset += 4;
                } else {
                    write_unaligned16( &argv_68k[offset],
                                       (unsigned short)param );
                    offset += 2;
                }
            }
            break;
        }
    }
    
    /* now call the OS.... */
    {
        STACK_START(unsigned char, stack, 12);
        ADD_TO_STACK4(stack, s, 0);
        ADD_TO_STACK4(stack, formatStr, 4);
        ADD_TO_STACK4(stack, argv_68k, 8);
        STACK_END(stack);
        result = (Int16)
            (twCall68KFunc)( twEmulState, 
                                 // NOT sysTrapStrPrintF !!!!
                                 PceNativeTrapNo(sysTrapStrVPrintF),
                                 stack, 12 );
    }

    return result;
} /* StrVPrintF */

/* Need to parse the format string */
static Int16 ftsl_StrPrintF( Char* s, const Char* formatStr, ... )
{
    unsigned long* inArgs = ((unsigned long*)&formatStr) + 1;

    return ftsl_StrVPrintF( s, formatStr, (_Palm_va_list)inArgs );
} /* StrPrintF */

void ftsl_sprintf(char *text, char *s, ...){
	va_list		argptr;

	va_start (argptr,s);
	ftsl_StrVPrintF (text, s, argptr);
	va_end (argptr);
}

static long ftsl_StrAToI (const Char* str)
{
	long result=0;
	int 	sign = 1;
	char	c;
	
	// First character can be a sign
	c = *str++;
	if (!c) return 0;
	if (c == '+') {sign = 1; c = *str++;}
	else if (c == '-') {sign = -1; c = *str++;}
	
	// Accumulate digits will we reach the end of the string
	while(c)
	   {
		if (c < '0' || c > '9') break;
		result = result * 10 + c -'0';
		c = *str++;
	   }

	return result * sign;
}

int ftsl_atoi(const char *s)
{
	return ftsl_StrAToI(s);
}

long ftsl_atol(const char *s)
{
	return ftsl_StrAToI(s);
}

long ftsl_labs(long a)
{
	return (a >= 0 ? a : -a);
}

void ftsl_exit(int i)
{
	ErrFatalDisplay("FTLib: Unhandled exception");
}

/******************************************************************/
/* qsort.c  --  Non-Recursive ANSI Quicksort function             */
/*                                                                */
/* Public domain by Raymond Gardner, Englewood CO  February 1991  */
/*                                                                */
/* Usage:                                                         */
/*     qsort(base, nbr_elements, width_bytes, compare_function);  */
/*        void *base;                                             */
/*        size_t nbr_elements, width_bytes;                       */
/*        int (*compare_function)(const void *, const void *);    */
/*                                                                */
/* Sorts an array starting at base, of length nbr_elements, each  */
/* element of size width_bytes, ordered via compare_function,     */
/* which is called as  (*compare_function)(ptr_to_element1,       */
/* ptr_to_element2) and returns < 0 if element1 < element2,       */
/* 0 if element1 = element2, > 0 if element1 > element2.          */
/* Most refinements are due to R. Sedgewick. See "Implementing    */
/* Quicksort Programs", Comm. ACM, Oct. 1978, and Corrigendum,    */
/* Comm. ACM, June 1979.                                          */
/******************************************************************/

/*
**  swap nbytes between a and b
*/
static void swap_chars(char *a, char *b, size_t nbytes)
{
   char tmp;
   do {
      tmp = *a; *a++ = *b; *b++ = tmp;
   } while ( --nbytes );
}

#define  SWAP(a, b)  (swap_chars((char *)(a), (char *)(b), size))
#define  COMP(a, b)  ((*comp)((void *)(a), (void *)(b)))
#define  T           7    /* subfiles of T or fewer elements will */
                          /* be sorted by a simple insertion sort */
                          /* Note!  T must be at least 3          */

void ftsl_qsort(void *basep, unsigned long nelems, unsigned long size, int (*comp)(const void *, const void *))
{
   char *stack[40], **sp;       /* stack and stack pointer        */
   char *i, *j, *limit;         /* scan and limit pointers        */
   size_t thresh;               /* size of T elements in bytes    */
   char *base;                  /* base pointer as char *         */
   
//if ((nelems == 3) && (size == 4)) Sys_PrintF("qsort in %lx %ld %ld %lx", basep, nelems, size, comp);
   base = (char *)basep;        /* set up char * base pointer     */
   thresh = T * size;           /* init threshold                 */
   sp = stack;                  /* init stack pointer             */
   limit = base + nelems * size;/* pointer past end of array      */
   for ( ;; ) {                 /* repeat until break...          */
      if ( limit - base > thresh ) {  /* if more than T elements  */
                                      /*   swap base with middle  */
         SWAP((((limit-base)/size)/2)*size+base, base);
         i = base + size;             /* i scans left to right    */
         j = limit - size;            /* j scans right to left    */
         if ( COMP(i, j) > 0 )        /* Sedgewick's              */
            SWAP(i, j);               /*    three-element sort    */
         if ( COMP(base, j) > 0 )     /*        sets things up    */
            SWAP(base, j);            /*            so that       */
         if ( COMP(i, base) > 0 )     /*      *i <= *base <= *j   */
            SWAP(i, base);            /* *base is pivot element   */
         for ( ;; ) {                 /* loop until break         */
            do                        /* move i right             */
               i += size;             /*        until *i >= pivot */
            while ( COMP(i, base) < 0 );
            do                        /* move j left              */
               j -= size;             /*        until *j <= pivot */
            while ( COMP(j, base) > 0 );
            if ( i > j )              /* if pointers crossed      */
               break;                 /*     break loop           */
            SWAP(i, j);       /* else swap elements, keep scanning*/
         }
         SWAP(base, j);         /* move pivot into correct place  */
         if ( j - base > limit - i ) {  /* if left subfile larger */
            sp[0] = base;             /* stack left subfile base  */
            sp[1] = j;                /*    and limit             */
            base = i;                 /* sort the right subfile   */
         } else {                     /* else right subfile larger*/
            sp[0] = i;                /* stack right subfile base */
            sp[1] = limit;            /*    and limit             */
            limit = j;                /* sort the left subfile    */
         }
         sp += 2;                     /* increment stack pointer  */
      } else {      /* else subfile is small, use insertion sort  */
//if ((nelems == 3) && (size == 4)) Sys_PrintF("1");
         for ( j = base, i = j+size; i < limit; j = i, i += size )
         {
//if ((nelems == 3) && (size == 4)) Sys_PrintF("j %lx size %ld", j, size);
            for ( ; COMP(j, j+size) > 0; j -= size ) {
               SWAP(j, j+size);
               if ( j == base )
                  break;
            }
         }
//if ((nelems == 3) && (size == 4)) Sys_PrintF("2");
         if ( sp != stack ) {         /* if any entries on stack  */
            sp -= 2;                  /* pop the base and limit   */
            base = sp[0];
            limit = sp[1];
         } else                       /* else stack empty, done   */
            break;
      }
   }
//if ((nelems == 3) && (size == 4)) Sys_PrintF("qsort out");
}
