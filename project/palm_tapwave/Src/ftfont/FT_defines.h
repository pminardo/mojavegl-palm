/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __FT_DEFINES_H_
#define __FT_DEFINES_H_

/**************************************************************
* FreeType Types
**************************************************************/

typedef DWORD		FTERR;
typedef void*			FTLIBRARY;
typedef void*			FTFACE;
typedef void*			FTGLYPHCACHE;

/**************************************************************
* FreeType Font Errors
**************************************************************/

#define FT_OK								0x00

/* Generic errors */
#define FTERR_CANNOT_OPEN_RESOURCE		0x01
#define FTERR_UNKNOWN_FILE_FORMAT		0x02
#define FTERR_INVALID_FILE_FORMAT			0x03
#define FTERR_INVALID_VERSION				0x04
#define FTERR_LOWER_MODULE_VERSION		0x05
#define FTERR_INVALID_ARGUMENT			0x06
#define FTERR_UNIMPLEMENTED_FEATURE		0x07
#define FTERR_INVALID_TABLE				0x08
#define FTERR_INVALID_OFFSET				0x09

/* Glyph/character errors */
#define FTERR_INVALID_GLYPH_INDEX			0x10
#define FTERR_INVALID_CHARACTER_CODE	0x11
#define FTERR_INVALID_GLYPH_FORMAT		0x12
#define FTERR_CANNOT_RENDER_GLYPH		0x13
#define FTERR_INVALID_OUTLINE			0x14
#define FTERR_INVALID_COMPOSITE			0x15
#define FTERR_TOO_MANY_HINTS			0x16
#define FTERR_INVALID_PIXEL_SIZE			0x17

/* Handle errors */
#define FTERR_INVALID_HANDLE			0x20
#define FTERR_INVALID_LIBRARY_HANDLE	0x21
#define FTERR_INVALID_DRIVER_HANDLE		0x22
#define FTERR_INVALID_FACE_HANDLE		0x23
#define FTERR_INVALID_SIZE_HANDLE		0x24
#define FTERR_INVALID_SLOT_HANDLE		0x25
#define FTERR_INVALID_CHARMAP_HANDLE	0x26
#define FTERR_INVALID_CACHE_HANDLE		0x27
#define FTERR_INVALID_STREAM_HANDLE	0x28

/* Driver errors */
#define FTERR_TOO_MANY_DRIVERS			0x30
#define FTERR_TOO_MANY_EXTENSIONS		0x31

/* Memory errors */
#define FTERR_OUT_OF_MEMORY			0x40
#define FTERR_UNLISTED_OBJECT			0x41

/* Stream errors */
#define FTERR_CANNOT_OPEN_STREAM		0x51
#define FTERR_INVALID_STREAM_SEEK		0x52
#define FTERR_INVALID_STREAM_SKIP		0x53
#define FTERR_INVALID_STREAM_READ		0x54
#define FTERR_INVALID_STREAM_OPERATION		0x55
#define FTERR_INVALID_FRAME_OPERATION		0x56
#define FTERR_NESTED_FRAME_ACCESS			0x57
#define FTERR_INVALID_FRAME_READ			0x58

/* Raster errors */
#define FTERR_RASTER_UNINITIALIZED		0x60
#define FTERR_RASTER_CORRUPTED		0x61
#define FTERR_RASTER_OVERFLOW			0x62
#define FTERR_RASTER_NEGATIVE_HEIGHT	0x63

/* Cache errors */
#define FTERR_TOO_MANY_CACHES			0x70

/* TrueType and SFNT errors */
#define FTERR_INVALID_OPCODE			0x80
#define FTERR_TOO_FEW_ARGUMENTS		0x81
#define FTERR_STACK_OVERFLOW			0x82
#define FTERR_CODE_OVERFLOW			0x83
#define FTERR_BAD_ARGUMENT				0x84
#define FTERR_DIVIDE_BY_ZERO			0x85
#define FTERR_INVALID_REFERENCE			0x86
#define FTERR_DEBUG_OPCODE			0x87
#define FTERR_ENDF_IN_EXEC_STREAM		0x88
#define FTERR_NESTED_DEFS				0x89
#define FTERR_INVALID_CODERANGE		0x8A
#define FTERR_EXECUTION_TOO_LONG		0x8B
#define FTERR_TOO_MANY_FUNCTION_DEFS	0x8C
#define FTERR_TOO_MANY_INSTRUCTION_DEFS	0x8D
#define FTERR_TABLE_MISSING				0x8E
#define FTERR_HORIZ_HEADER_MISSING		0x8F
#define FTERR_LOCATIONS_MISSING			0x90
#define FTERR_NAME_TABLE_MISSING		0x91
#define FTERR_CMAP_TABLE_MISSING		0x92
#define FTERR_HMTX_TABLE_MISSING		0x93
#define FTERR_POST_TABLE_MISSING		0x94
#define FTERR_INVALID_HORIZ_METRICS		0x95
#define FTERR_INVALID_CHARMAP_FORMAT	0x96
#define FTERR_INVALID_PPEM				0x97
#define FTERR_INVALID_VERT_METRICS		0x98
#define FTERR_COULD_NOT_FIND_CONTEXT	0x99
#define FTERR_INVALID_POST_TABLE_FORMAT	0x9A
#define FTERR_INVALID_POST_TABLE		0x9B

/* CFF, CID, and Type 1 errors */
#define FTERR_SYNTAX_ERROR				0xA0
#define FTERR_STACK_UNDERFLOW			0xA1
#define FTERR_IGNORE					0xA2

/* BDF errors */
#define FTERR_MISSING_STARTFONT_FIELD	0xB0
#define FTERR_MISSING_FONT_FIELD		0xB1
#define FTERR_MISSING_SIZE_FIELD			0xB2
#define FTERR_MISSING_CHARS_FIELD		0xB3
#define FTERR_MISSING_STARTCHAR_FIELD	0xB4
#define FTERR_MISSING_ENCODING_FIELD	0xB5
#define FTERR_MISSING_BBX_FIELD			0xB6

#endif // __FT_DEFINES_H_