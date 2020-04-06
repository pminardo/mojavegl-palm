/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
*
* Written by Parker Minardo
**************************************************************/

#include "MojaveGL.h"
#include "math/M_math.h"

/**************************************************************
* Internal Functions
**************************************************************/

// Return cosine of x.
DOUBLE mglmath::cos(DOUBLE x)
{
	return M_cos(x);
}

// Return sine of x.
DOUBLE mglmath::sin(DOUBLE x)
{
	return M_sin(x);
}

// Return sqrt of x.
DOUBLE mglmath::sqrt(DOUBLE x)
{
	return M_sqrt(x);
}