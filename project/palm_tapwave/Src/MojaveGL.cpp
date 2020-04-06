/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"

#include "string/S_string.h"

#include "ftfont/FT_defines.h"
#include "ftfont/FT_font.h"

/**************************************************************
* Internal Functions
**************************************************************/

// CMgl constructor
CMgl::CMgl()
{
	// Initialize the codepage globals for use. This MUST be done
	// in order to convert MBCS <-> Unicode (used for DrawText).
	S_InitCodePages();
	
	// Clear the globals
	mgl_memset(&m_globals, 0, sizeof(mglinternal::MGLGLOBALS));
}

// CMgl destructor
CMgl::~CMgl()
{
	FreeInstance();
}

// Initialize the MojaveGL instance.
HRESULT CMgl::Initialize()
{
	FTERR ftError;
	FTLIBRARY ftLibrary;
	HRESULT hr;
	
	// If the library is already initialized, bail out.
	if (IsInitialized())
		return MGLERR_ALREADYINITIALIZED;
	
	// Initialize the FreeType library
	ftError = FT_OpenLibrary(&ftLibrary);
	
	if (ftError != FT_OK)
	{
		// Translate the FreeType error
		switch (ftError)
		{
			case FTERR_INVALID_ARGUMENT:
				hr = MGLERR_INVALIDPARAMS;
				break;
		
			case FTERR_OUT_OF_MEMORY:
				hr = MGLERR_OUTOFMEMORY;
				break;
				
			default:
				hr = E_FAIL;
				break;
		}
	
		return hr;
	}
	
	// Store the FreeType instance in the CMgl globals
	m_globals.pFtLibrary = ftLibrary;

	return MGL_OK;
}

BOOL CMgl::IsInitialized()
{
	// The MojaveGL instance is initialized
	// if we have a FreeType instance.
	return (m_globals.pFtLibrary != NULL);
}

HRESULT CMgl::FreeInstance()
{
	FTLIBRARY ftLibrary;

	// If the library is not already initialized, bail out.
	if (!IsInitialized())
		return MGLERR_NOTINITIALIZED;
		
	// Close the FreeType instance.
	ftLibrary = (FTLIBRARY)m_globals.pFtLibrary;
	FT_CloseLibrary(ftLibrary);
	
	// Clear the globals
	mgl_memset(&m_globals, 0, sizeof(mglinternal::MGLGLOBALS));
		
	return MGL_OK;	
}