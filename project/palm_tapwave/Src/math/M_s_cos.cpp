/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/

#include "MojaveGLDefines.h"
#include "M_math.h"

/**************************************************************
* Internal Functions
**************************************************************/

double M_cos(double x)
{
	double y[2],z=0.0;
	int n, ix;

    /* High word of x. */
	ix = __HI(x);

    /* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3fe921fb) return M__kernel_cos(x,z);

    /* _mgl2d_cos(Inf or NaN) is NaN */
	else if (ix>=0x7ff00000) return x-x;

    /* argument reduction needed */
	else {
	    n = M__ieee754_rem_pio2(x,y);
	    switch(n&3) {
		case 0: return  M__kernel_cos(y[0],y[1]);
		case 1: return -M__kernel_sin(y[0],y[1],1);
		case 2: return -M__kernel_cos(y[0],y[1]);
		default:
		        return  M__kernel_sin(y[0],y[1],1);
	    }
	}
}
