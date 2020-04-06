/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
*
* NOTE: We could just use FreeType's cache
* manager, but it's easiest to make our own!
**************************************************************/

#include "MojaveGL.h"

#include <ft2build.h>
#include <freetype/freetype.h>

#include "FT_glyphcache.h"

/**************************************************************
* Helper Functions
**************************************************************/

namespace FTGlyphCache {

	inline
	GLYPHCACHENODE* CreateNode(DWORD dwGlyphIndex, FT_GlyphSlot ftGlyphSlot)
	{
		GLYPHCACHENODE *pNode;
		void *pBmpBuffer;
		
		// Create the node
		pNode = (GLYPHCACHENODE*)ftsl_malloc(sizeof(GLYPHCACHENODE));
		if (!pNode) return NULL;
		
		// If there's a bitmap to be stored, allocate it.
		if (ftGlyphSlot->bitmap.buffer)
		{
			LONG nPitch; 
			DWORD dwBmpSize;
			
			nPitch = ftGlyphSlot->bitmap.pitch;
			if (nPitch < 0) nPitch = -nPitch;	// ensure pitch is positive
			
			// Compute the number of bytes the cached bitmap will occupy
			dwBmpSize = (DWORD)(nPitch * ftGlyphSlot->bitmap.rows);
			
			// Allocate memory for the bitmap
			pBmpBuffer = ftsl_malloc(dwBmpSize);
			if (!pBmpBuffer) goto error;	// bail out!
			
			// Copy the bitmap data
			ftsl_memcpy(pBmpBuffer, ftGlyphSlot->bitmap.buffer, dwBmpSize);
		}
		else
			pBmpBuffer = NULL;
	
		// Initialize the node
		pNode->dwRefCount = 0;
		pNode->dwGlyphIndex = dwGlyphIndex;
		
		pNode->pNext = NULL;
		pNode->pPrevious = NULL;
		
		// Initialize the node data
		pNode->data.dwWidth = ftGlyphSlot->bitmap.width;
		pNode->data.dwHeight = ftGlyphSlot->bitmap.rows;
		pNode->data.nLeft = ftGlyphSlot->bitmap_left;
		pNode->data.nTop = ftGlyphSlot->bitmap_top;
		
		pNode->data.xAdvance = ftGlyphSlot->advance.x;
		pNode->data.yAdvance = ftGlyphSlot->advance.y;
		
		pNode->data.pixelMode = ftGlyphSlot->bitmap.pixel_mode;
		pNode->data.maxGrays = ftGlyphSlot->bitmap.num_grays;
		
		pNode->data.pitch = ftGlyphSlot->bitmap.pitch;
		pNode->data.pBuffer = pBmpBuffer;
	
		return pNode;
		
		error:
		
		// Free the node
		ftsl_free(pNode);
		
		return NULL;
	}
	
	inline
	GLYPHCACHENODE* FindNodeByGlyphIndex(GLYPHCACHENODE *pHead, DWORD dwGlyphIndex)
	{
		GLYPHCACHENODE *pNode;
		
		pNode = pHead;
		while (pNode)
		{
			// If the glyph index matches, we found it!
			if (pNode->dwGlyphIndex == dwGlyphIndex)
				break;
		
			// Advance to next node in the list
			pNode = pNode->pNext;
		}
		
		return pNode; 
	}

	inline
	void LinkNodePrepend(GLYPHCACHE *pCache, GLYPHCACHENODE *pNode)
	{
		// If there are no nodes in the list, it's easy.
		if (!pCache->pHead)
		{
			// Set head and tail pointers
			pCache->pHead = pNode;
			pCache->pTail = pNode;
		}
		else
		{
			// Link the head node to the new node
			pCache->pHead->pPrevious = pNode;
			
			// Link the new node to the head node
			pNode->pNext = pCache->pHead;
			
			// Update head pointer since the node is
			// now at the beginning of the list.
			pCache->pHead = pNode;
		}
	}

	inline
	void UnlinkNode(GLYPHCACHE *pCache, GLYPHCACHENODE *pNode)
	{
		// If the node is at the tail of the list, fixup the tail.
		if (pCache->pTail == pNode)
			pCache->pTail = pNode->pPrevious;
	
		// If the node is the head of the list, then it's easy.
		if (pCache->pHead == pNode)
		{
			if (pNode->pNext)
				pNode->pNext->pPrevious = NULL;
				
			pCache->pHead = pNode->pNext;
		}
		else
		{
			GLYPHCACHENODE *pBefore, *pAfter;
			
			pBefore = pNode->pPrevious;
			pAfter = pNode->pNext;
			
			/* if (pBefore) */
				pBefore->pNext = pAfter;
			
			if (pAfter)
				pAfter->pPrevious = 	pBefore;	
		}
		
		pNode->pNext = NULL;
		pNode->pPrevious = NULL;
	}
	
	inline
	void DeleteNode(GLYPHCACHENODE *pNode)
	{
		// Free the entry's bitmap buffer, if there is one
		if (pNode->data.pBuffer)
		{
			ftsl_free(pNode->data.pBuffer);
		}
		
		// Free the current entry
		ftsl_free(pNode);
	}

	inline
	void DeleteAllNodes(GLYPHCACHENODE *pHead)
	{
		GLYPHCACHENODE *pNode, *pNextNode;
		
		pNode = pHead;
		while (pNode)
		{
			// Store the next node pointer since we
			// won't be able to access it once we free
			// the current node.
			pNextNode = pNode->pNext;
			
			// Delete the current node
			DeleteNode(pNode);
			
			// Advance to the next node
			pNode = pNextNode;
		}
	}
}

/**************************************************************
* Internal Functions
**************************************************************/

#ifdef _PRGM_SECT
#pragma mark ------------ Glyph Cache Functions --------------
#endif

// Create a new glyph cache for a font face.
FT_Error FT_CreateGlyphCache(DWORD dwMaxItems, FT_CacheHandle *phCache /*OUT*/)
{
	GLYPHCACHE *pCache;
	
	// Verify parameters
	if (dwMaxItems == 0 || !phCache)
		return FT_Err_Invalid_Argument;
	
	// Allocate the cache object
	pCache = (GLYPHCACHE*)ftsl_malloc(sizeof(GLYPHCACHE));
	if (!pCache) return FT_Err_Out_Of_Memory;
	
	// Init the cache object
	pCache->dwMaxItems = dwMaxItems;
	
	pCache->dwNumItems = 0;
	pCache->pHead = NULL;
	pCache->pTail = NULL;
	
	// Return cache object
	*phCache = (FT_CacheHandle)pCache;

	return FT_Err_Ok;
}

// Add a glyph entry to the cache from a FT_GlyphSlot. This function
// stores a copy of the glyphslot bitmap in the glyph cache. If the maxItems
// limit has been reached, this function removes the last glyph entry having
// a reference counter of zero before prepending the new entry.
FT_Error FT_AddGlyphCacheEntry(FT_CacheHandle hCache, DWORD dwGlyphIndex, FT_GlyphSlot ftGlyphSlot)
{
	GLYPHCACHE *pCache;
	GLYPHCACHENODE *pEntry;
	
	// Verify parameters
	if (!hCache) return FT_Err_Invalid_Argument;

	// Get a pointer to the cache object
	pCache = (GLYPHCACHE*)hCache;
	
	// If we need to ditch the last entry in the cache, do it!
	if (pCache->dwNumItems >= pCache->dwMaxItems)
	{
		GLYPHCACHENODE *pNodeLast;
		BOOL bFound = FALSE;
	
		// Unlink the last node in the list whose refCount is zero.
		pNodeLast = pCache->pTail;
		while (pNodeLast)
		{
			if (pNodeLast->dwRefCount == 0)
				break;
			
			// Skip to the preceding node
			pNodeLast = pNodeLast->pPrevious;
		}
		
		// If no nodes having a refCount of zero were found, bail out.
		if (!pNodeLast) return FT_Err_GlyphCacheFull;
		
		// Remove the last node
		FTGlyphCache::UnlinkNode(pCache, pNodeLast);
		FTGlyphCache::DeleteNode(pNodeLast);
		--pCache->dwNumItems;
	}
	
	// Allocate a new cache entry
	pEntry = FTGlyphCache::CreateNode(dwGlyphIndex, ftGlyphSlot);
	if (!pEntry) return FT_Err_Out_Of_Memory;
	
	// Prepend the cache entry
	FTGlyphCache::LinkNodePrepend(pCache, pEntry);
	
	// Increment item count
	++pCache->dwNumItems;
	
	return FT_Err_Ok;
}

// Lookup a glyph entry. If successful, this function moves the entry
// to the beginning of the glyph cache list and increments its reference count.
FT_Error FT_LookupGlyphCacheEntry(FT_CacheHandle hCache, DWORD dwGlyphIndex, GLYPHCACHEDATA *pData /*OUT*/)
{
	GLYPHCACHE *pCache;
	GLYPHCACHENODE *pEntry;
	
	// Verify parameters
	if (!hCache || !pData) return FT_Err_Invalid_Argument;

	// Get a pointer to the cache object
	pCache = (GLYPHCACHE*)hCache;
	
	// Find the entry by its glyph index
	pEntry = FTGlyphCache::FindNodeByGlyphIndex(pCache->pHead, dwGlyphIndex);
	if (!pEntry) return FT_Err_GlyphCacheEntryNotFound;
	
	// Increment refCount
	++pEntry->dwRefCount;
	
	// OPTIMIZATION: If the node isn't at the head of the list,
	// unlink it and move it to the beginning.
	if (pCache->pHead != pEntry)
	{
		FTGlyphCache::UnlinkNode(pCache, pEntry);
		FTGlyphCache::LinkNodePrepend(pCache, pEntry);
	}
	
	// Return cache data
	ftsl_memcpy(pData, &pEntry->data, sizeof(GLYPHCACHEDATA));
	
	return FT_Err_Ok;
}

// Release a glyph entry by decrementing its reference count.
FT_Error FT_ReleaseGlyphCacheEntry(FT_CacheHandle hCache, DWORD dwGlyphIndex)
{
	GLYPHCACHE *pCache;
	GLYPHCACHENODE *pEntry;
	
	// Verify parameters
	if (!hCache) return FT_Err_Invalid_Argument;

	// Get a pointer to the cache object
	pCache = (GLYPHCACHE*)hCache;
	
	// Find the entry by its glyph index
	pEntry = FTGlyphCache::FindNodeByGlyphIndex(pCache->pHead, dwGlyphIndex);
	if (!pEntry) return FT_Err_GlyphCacheEntryNotFound;
	
	// Validate the refCount
	if (pEntry->dwRefCount == 0)
		return FT_Err_GlyphCacheEntryBadRefCount;
		
	// Decrement refCount
	--pEntry->dwRefCount;

	return FT_Err_Ok;
}

// Remove a glyph entry.
FT_Error FT_RemoveGlyphCacheEntry(FT_CacheHandle hCache, DWORD dwGlyphIndex)
{
	GLYPHCACHE *pCache;
	GLYPHCACHENODE *pEntry;
	
	// Verify parameters
	if (!hCache) return FT_Err_Invalid_Argument;

	// Get a pointer to the cache object
	pCache = (GLYPHCACHE*)hCache;
	
	// Find the entry by its glyph index
	pEntry = FTGlyphCache::FindNodeByGlyphIndex(pCache->pHead, dwGlyphIndex);
	if (!pEntry) return FT_Err_GlyphCacheEntryNotFound;
	
	// Remove the entry from the cache
	FTGlyphCache::UnlinkNode(pCache, pEntry);
	FTGlyphCache::DeleteNode(pEntry);
	--pCache->dwNumItems;

	return FT_Err_Ok;
}

// Remove all glyph entries in the glyph cache.
FT_Error FT_EmptyGlyphCache(FT_CacheHandle hCache)
{
	GLYPHCACHE *pCache;
	
	// Verify parameters
	if (!hCache) return FT_Err_Invalid_Argument;

	// Get a pointer to the cache object
	pCache = (GLYPHCACHE*)hCache;
	
	// Remove all nodes from the cache
	FTGlyphCache::DeleteAllNodes(pCache->pHead);
	
	pCache->dwNumItems = 0;
	pCache->pHead = NULL;
	pCache->pTail = NULL;
	
	return FT_Err_Ok;
}

// Delete a glyph cache.
FT_Error FT_DeleteGlyphCache(FT_CacheHandle hCache)
{
	GLYPHCACHE *pCache;
	
	// Verify parameters
	if (!hCache) return FT_Err_Invalid_Argument;

	// Get a pointer to the cache object
	pCache = (GLYPHCACHE*)hCache;
	
	// Remove all nodes from the cache
	FTGlyphCache::DeleteAllNodes(pCache->pHead);
	
	// Free the cache object
	ftsl_free(pCache);

	return FT_Err_Ok;
}