/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __FT_GLYPHCACHE_H_
#define __FT_GLYPHCACHE_H_

typedef struct _GLYPHCACHEDATA {

	DWORD dwWidth;
	DWORD dwHeight;
	LONG nLeft;
	LONG nTop;
	
	LONG xAdvance;
	LONG yAdvance;
	
	BYTE pixelMode;	// monochrome or gray
	WORD maxGrays;	// maximum gray level value (in the range 1 to 255)

	LONG pitch;		// bytes per bitmap line; may be positive or negative
	void *pBuffer;		// bitmap pixels

} GLYPHCACHEDATA;

// The following structure represents a node in the
// glyph cache.
typedef struct _GLYPHCACHENODE {
	DWORD dwRefCount;	// reference counter
	DWORD dwGlyphIndex;	// FreeType glyph index
	GLYPHCACHEDATA data;
	_GLYPHCACHENODE *pNext;
	_GLYPHCACHENODE *pPrevious;
} GLYPHCACHENODE;

// The following structure represents a glyph cache.
typedef struct _GLYPHCACHE {
	DWORD dwMaxItems;		// max items in glyph cache
	
	DWORD dwNumItems;		// num items in the list
	GLYPHCACHENODE *pHead;	// ptr to first node (stored in MRU order)
	GLYPHCACHENODE *pTail;	// ptr to last node
} GLYPHCACHE;

typedef void*		FT_CacheHandle; // handle to a glyph cache

// IMPORTANT: The following constant(s) must not conflict with those
// defined in fterrdef.h!!!
#define FT_Err_GlyphCacheFull				0x10000000L
#define FT_Err_GlyphCacheEntryNotFound		0x10000001L
#define FT_Err_GlyphCacheEntryBadRefCount	0x10000002L

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************
* Glyph Cache Functions
**************************************************************/

// Create a new glyph cache for a font face.
FT_Error FT_CreateGlyphCache(DWORD dwMaxItems, FT_CacheHandle *phCache /*OUT*/);

// Add a glyph entry to the cache from a FT_GlyphSlot. This function
// stores a copy of the glyphslot bitmap in the glyph cache. If the maxItems
// limit has been reached, this function removes the last glyph entry having
// a reference counter of zero before prepending the new entry.
FT_Error FT_AddGlyphCacheEntry(FT_CacheHandle hCache, DWORD dwGlyphIndex, FT_GlyphSlot ftGlyphSlot);

// Lookup a glyph entry. If successful, this function moves the entry
// to the beginning of the glyph cache list and increments its reference count.
FT_Error FT_LookupGlyphCacheEntry(FT_CacheHandle hCache, DWORD dwGlyphIndex, GLYPHCACHEDATA *pData /*OUT*/);

// Release a glyph entry by decrementing its reference count.
FT_Error FT_ReleaseGlyphCacheEntry(FT_CacheHandle hCache, DWORD dwGlyphIndex);

// Remove a glyph entry.
FT_Error FT_RemoveGlyphCacheEntry(FT_CacheHandle hCache, DWORD dwGlyphIndex);

// Remove all glyph entries in the glyph cache.
FT_Error FT_EmptyGlyphCache(FT_CacheHandle hCache);

// Delete a glyph cache.
FT_Error FT_DeleteGlyphCache(FT_CacheHandle hCache);

#ifdef __cplusplus
}
#endif

#endif // __FT_GLYPHCACHE_H_