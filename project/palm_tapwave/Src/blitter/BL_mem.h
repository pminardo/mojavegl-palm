/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __BL_MEMCPY_H_
#define __BL_MEMCPY_H_

/**************************************************************
* Macros/Constants
**************************************************************/

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE_16    (sizeof (short) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE_16 (sizeof (short))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

// Highly optimized memcpy for internal MojaveGL2d use

/*
FUNCTION
        <<memcpy>>---copy memory regions

ANSI_SYNOPSIS
        #include <string.h>
        void* memcpy(void *<[out]>, const void *<[in]>, size_t <[n]>);

TRAD_SYNOPSIS
        void *memcpy(<[out]>, <[in]>, <[n]>
        void *<[out]>;
        void *<[in]>;
        size_t <[n]>;

DESCRIPTION
        This function copies <[n]> bytes from the memory region
        pointed to by <[in]> to the memory region pointed to by
        <[out]>.

        If the regions overlap, the behavior is undefined.

RETURNS
        <<memcpy>> returns a pointer to the first byte of the <[out]>
        region.

PORTABILITY
<<memcpy>> is ANSI C.

<<memcpy>> requires no supporting OS subroutines.

QUICKREF
        memcpy ansi pure
	*/
inline void * BL_memcpy (
	void *dst0,
	const void *src0,
	unsigned long len0)
{
  char *dst = (char*)dst0;
  const char *src = (const char*)src0;
  long *aligned_dst;
  const long *aligned_src;
  int   len =  len0;

  /* If the size is small, or either SRC or DST is unaligned,
     then punt into the byte copy loop.  This should be rare.  */
  if (!TOO_SMALL(len) && !UNALIGNED (src, dst))
    {
      aligned_dst = (long*)dst;
      aligned_src = (long*)src;

      /* Copy 4X long words at a time if possible.  */
      while (len >= BIGBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          len -= BIGBLOCKSIZE;
        }

      /* Copy one long word at a time if possible.  */
      while (len >= LITTLEBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          len -= LITTLEBLOCKSIZE;
        }

       /* Pick up any residual with a byte copier.  */
      dst = (char*)aligned_dst;
      src = (char*)aligned_src;
    }

  while (len--)
    *dst++ = *src++;

  return dst0;
}

// Performs a 32-bit memcpy, assuming the caller
// provides properly alignment!
inline void * BL_memcpy_32bit (
	void *dst0,
	const void *src0,
	unsigned long len)
{
  register long *aligned_dst;
  register const long *aligned_src;
  
  aligned_dst = (long*)dst0;
  aligned_src = (long*)src0;
        
      /* Copy 4X long words at a time if possible.  */
      while (len >= BIGBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          len -= BIGBLOCKSIZE;
        }

      /* Copy one long word at a time if possible.  */
      while (len >= LITTLEBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          len -= LITTLEBLOCKSIZE;
        }

  return dst0;
}


#endif // __BL_MEMCPY_H_