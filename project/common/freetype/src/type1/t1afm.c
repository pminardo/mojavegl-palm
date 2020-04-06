/***************************************************************************/
/*                                                                         */
/*  t1afm.c                                                                */
/*                                                                         */
/*    AFM support for Type 1 fonts (body).                                 */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>
#include "t1afm.h"
#include "t1errors.h"
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_TYPE1_TYPES_H


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t1afm


  FT_LOCAL_DEF( void )
  T1_Done_Metrics( FT_Memory  memory,
                   T1_AFM*    afm )
  {
    FT_FREE( afm->kern_pairs );
    afm->num_pairs = 0;
    FT_FREE( afm );
  }


#undef  IS_KERN_PAIR
#define IS_KERN_PAIR( p )  ( p[0] == 'K' && p[1] == 'P' )

#define IS_ALPHANUM( c )  ( ft_isalnum( c ) || \
                            c == '_'        || \
                            c == '.'        )


  /* read a glyph name and return the equivalent glyph index */
  static FT_UInt
  afm_atoindex( FT_Byte**  start,
                FT_Byte*   limit,
                T1_Font    type1 )
  {
    FT_Byte*    p = *start;
    FT_PtrDist  len;
    FT_UInt     result = 0;
    char        temp[64];


    /* skip whitespace */
    while ( p < limit                                             &&
            ( *p == ' ' || *p == '\t' || *p == ':' || *p == ';' ) )
      p++;
    *start = p;

    /* now, read glyph name */
    while ( p < limit && IS_ALPHANUM( *p ) )
      p++;

    len = p - *start;

    if ( len > 0 && len < 64 )
    {
      FT_Int  n;


      /* copy glyph name to intermediate array */
      FT_MEM_COPY( temp, *start, len );
      temp[len] = 0;

      /* lookup glyph name in face array */
      for ( n = 0; n < type1->num_glyphs; n++ )
      {
        char*  gname = (char*)type1->glyph_names[n];


        if ( gname && gname[0] == temp[0] && ft_strcmp( gname, temp ) == 0 )
        {
          result = n;
          break;
        }
      }
    }
    *start = p;
    return result;
  }


  /* read an integer */
  static int
  afm_atoi( FT_Byte**  start,
            FT_Byte*   limit )
  {
    FT_Byte*  p    = *start;
    int       sum  = 0;
    int       sign = 1;


    /* skip everything that is not a number */
    while ( p < limit && !ftsl_isdigit( *p ) )
    {
      sign = 1;
      if ( *p == '-' )
        sign = -1;

      p++;
    }

    while ( p < limit && ftsl_isdigit( *p ) )
    {
      sum = sum * 10 + ( *p - '0' );
      p++;
    }
    *start = p;

    return sum * sign;
  }


#undef  KERN_INDEX
#define KERN_INDEX( g1, g2 )  ( ( (FT_ULong)g1 << 16 ) | g2 )


  /* compare two kerning pairs */
  FT_CALLBACK_DEF( int )
  compare_kern_pairs( const void*  a,
                      const void*  b )
  {
    T1_Kern_Pair*  pair1 = (T1_Kern_Pair*)a;
    T1_Kern_Pair*  pair2 = (T1_Kern_Pair*)b;

    FT_ULong  index1 = KERN_INDEX( pair1->glyph1, pair1->glyph2 );
    FT_ULong  index2 = KERN_INDEX( pair2->glyph1, pair2->glyph2 );


    return (int)( index1 - index2 );
  }


  /* parse an AFM file -- for now, only read the kerning pairs */
  static FT_Error
  T1_Read_AFM( FT_Face    t1_face,
               FT_Stream  stream )
  {
    FT_Error       error = T1_Err_Ok;
    FT_Memory      memory = stream->memory;
    FT_Byte*       start;
    FT_Byte*       limit;
    FT_Byte*       p;
    FT_Int         count = 0;
    T1_Kern_Pair*  pair;
    T1_Font        type1 = &((T1_Face)t1_face)->type1;
    T1_AFM*        afm   = 0;


    start = (FT_Byte*)stream->cursor;
    limit = (FT_Byte*)stream->limit;
    p     = start;

    /* we are now going to count the occurences of `KP' or `KPX' in */
    /* the AFM file                                                 */
    count = 0;
    for ( p = start; p < limit - 3; p++ )
    {
      if ( IS_KERN_PAIR( p ) )
        count++;
    }

    /* Actually, kerning pairs are simply optional! */
    if ( count == 0 )
      goto Exit;

    /* allocate the pairs */
    if ( FT_NEW( afm ) || FT_NEW_ARRAY( afm->kern_pairs, count ) )
      goto Exit;

    /* now, read each kern pair */
    pair           = afm->kern_pairs;
    afm->num_pairs = count;

    /* save in face object */
    ((T1_Face)t1_face)->afm_data = afm;

    t1_face->face_flags |= FT_FACE_FLAG_KERNING;

    for ( p = start; p < limit - 3; p++ )
    {
      if ( IS_KERN_PAIR( p ) )
      {
        FT_Byte*  q;


        /* skip keyword (KP or KPX) */
        q = p + 2;
        if ( *q == 'X' )
          q++;

        pair->glyph1    = afm_atoindex( &q, limit, type1 );
        pair->glyph2    = afm_atoindex( &q, limit, type1 );
        pair->kerning.x = afm_atoi( &q, limit );

        pair->kerning.y = 0;
        if ( p[2] != 'X' )
          pair->kerning.y = afm_atoi( &q, limit );

        pair++;
      }
    }

    /* now, sort the kern pairs according to their glyph indices */
    ft_qsort( afm->kern_pairs, count, sizeof ( T1_Kern_Pair ),
              compare_kern_pairs );

  Exit:
    if ( error )
      FT_FREE( afm );

    return error;
  }


#define LITTLE_ENDIAN_USHORT( p )  (FT_UShort)( ( (p)[0]       ) | \
                                                ( (p)[1] <<  8 ) )

#define LITTLE_ENDIAN_UINT( p )  (FT_UInt)( ( (p)[0]       ) | \
                                            ( (p)[1] <<  8 ) | \
                                            ( (p)[2] << 16 ) | \
                                            ( (p)[3] << 24 ) )


  /* parse a PFM file -- for now, only read the kerning pairs */
  static FT_Error
  T1_Read_PFM( FT_Face    t1_face,
               FT_Stream  stream )
  {
    FT_Error       error = T1_Err_Ok;
    FT_Memory      memory = stream->memory;
    FT_Byte*       start;
    FT_Byte*       limit;
    FT_Byte*       p;
    FT_Int         kern_count = 0;
    T1_Kern_Pair*  pair;
    T1_AFM*        afm = 0;
    FT_Int         width_table_length;
    FT_CharMap     oldcharmap;
    FT_CharMap     charmap;
    FT_Int         n;


    start = (FT_Byte*)stream->cursor;
    limit = (FT_Byte*)stream->limit;
    p     = start;

    /* Figure out how long the width table is.          */
    /* This info is a little-endian short at offset 99. */
    p = start + 99;
    if ( p + 2 > limit )
    {
      error = T1_Err_Unknown_File_Format;
      goto Exit;
    }
    width_table_length = LITTLE_ENDIAN_USHORT( p );

    p += 18 + width_table_length;
    if ( p + 0x12 > limit || LITTLE_ENDIAN_USHORT( p ) < 0x12 )
      /* extension table is probably optional */
      goto Exit;

    /* Kerning offset is 14 bytes from start of extensions table. */
    p += 14;
    p = start + LITTLE_ENDIAN_UINT( p );
    if ( p + 2 > limit )
    {
      error = T1_Err_Unknown_File_Format;
      goto Exit;
    }

    kern_count = LITTLE_ENDIAN_USHORT( p );
    p += 2;
    if ( p + 4 * kern_count > limit )
    {
      error = T1_Err_Unknown_File_Format;
      goto Exit;
    }

    /* Actually, kerning pairs are simply optional! */
    if ( kern_count == 0 )
      goto Exit;

    /* allocate the pairs */
    if ( FT_NEW( afm ) || FT_NEW_ARRAY( afm->kern_pairs, kern_count ) )
      goto Exit;

    /* save in face object */
    ((T1_Face)t1_face)->afm_data = afm;

    t1_face->face_flags |= FT_FACE_FLAG_KERNING;

    /* now, read each kern pair */
    pair           = afm->kern_pairs;
    afm->num_pairs = kern_count;
    limit          = p + 4 * kern_count;

    /* PFM kerning data are stored by encoding rather than glyph index, */
    /* so find the PostScript charmap of this font and install it       */
    /* temporarily.  If we find no PostScript charmap, then just use    */
    /* the default and hope it is the right one.                        */
    oldcharmap = t1_face->charmap;
    charmap    = NULL;

    for ( n = 0; n < t1_face->num_charmaps; n++ )
    {
      charmap = t1_face->charmaps[n];
      /* check against PostScript pseudo platform */
      if ( charmap->platform_id == 7 )
      {
        error = FT_Set_Charmap( t1_face, charmap );
        if ( error )
          goto Exit;
        break;
      }
    }

    /* Kerning info is stored as:             */
    /*                                        */
    /*   encoding of first glyph (1 byte)     */
    /*   encoding of second glyph (1 byte)    */
    /*   offset (little-endian short)         */
    for ( ; p < limit ; p+=4 )
    {
      pair->glyph1 = FT_Get_Char_Index( t1_face, p[0] );
      pair->glyph2 = FT_Get_Char_Index( t1_face, p[1] );

      pair->kerning.x = (FT_Short)LITTLE_ENDIAN_USHORT(p + 2);
      pair->kerning.y = 0;

      pair++;
    }

    if ( oldcharmap != NULL )
      error = FT_Set_Charmap( t1_face, oldcharmap );
    if ( error )
      goto Exit;

    /* now, sort the kern pairs according to their glyph indices */
    ft_qsort( afm->kern_pairs, kern_count, sizeof ( T1_Kern_Pair ),
              compare_kern_pairs );

  Exit:
    if ( error )
      FT_FREE( afm );

    return error;
  }


  /* parse a metrics file -- either AFM or PFM depending on what */
  /* it turns out to be                                          */
  FT_LOCAL_DEF( FT_Error )
  T1_Read_Metrics( FT_Face    t1_face,
                   FT_Stream  stream )
  {
    FT_Error  error;
    FT_Byte*  start;


    if ( FT_FRAME_ENTER( stream->size ) )
      return error;

    start = (FT_Byte*)stream->cursor;

    if ( stream->size >= ft_strlen( "StartFontMetrics" )    &&
         ft_strncmp( (const char*)start, "StartFontMetrics",
                     ft_strlen( "StartFontMetrics" ) ) == 0 )
      error = T1_Read_AFM( t1_face, stream );

    else if ( stream->size > 6                                &&
              start[0] == 0x00 && start[1] == 0x01            &&
              LITTLE_ENDIAN_UINT( start + 2 ) == stream->size )
      error = T1_Read_PFM( t1_face, stream );

    else
      error = T1_Err_Unknown_File_Format;

    FT_FRAME_EXIT();

    return error;
  }


  /* find the kerning for a given glyph pair */
  FT_LOCAL_DEF( void )
  T1_Get_Kerning( T1_AFM*     afm,
                  FT_UInt     glyph1,
                  FT_UInt     glyph2,
                  FT_Vector*  kerning )
  {
    T1_Kern_Pair  *min, *mid, *max;
    FT_ULong      idx = KERN_INDEX( glyph1, glyph2 );


    /* simple binary search */
    min = afm->kern_pairs;
    max = min + afm->num_pairs - 1;

    while ( min <= max )
    {
      FT_ULong  midi;


      mid  = min + ( max - min ) / 2;
      midi = KERN_INDEX( mid->glyph1, mid->glyph2 );

      if ( midi == idx )
      {
        *kerning = mid->kerning;
        return;
      }

      if ( midi < idx )
        min = mid + 1;
      else
        max = mid - 1;
    }

    kerning->x = 0;
    kerning->y = 0;
  }


/* END */
