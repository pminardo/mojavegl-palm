/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/

#include "MojaveGLDefines.h"
#include "M_math.h"

/**************************************************************
* Internal Functions
**************************************************************/

double M_fabs(double x)
{
	__HI(x) &= 0x7fffffff;
        return x;
}