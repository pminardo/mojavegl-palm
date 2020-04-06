/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/

#include "MojaveGLDefines.h"
#include "M_math.h"

/**************************************************************
* Internal Functions
**************************************************************/

double M_copysign(double x, double y)
{
	__HI(x) = (__HI(x)&0x7fffffff)|(__HI(y)&0x80000000);
        return x;
}