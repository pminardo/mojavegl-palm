/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/

#include "MojaveGLDefines.h"
#include "M_math.h"

/**************************************************************
* Internal Functions
**************************************************************/

double M_sqrt(double x)		/* wrapper sqrt */
{
	return M__ieee754_sqrt(x);
}