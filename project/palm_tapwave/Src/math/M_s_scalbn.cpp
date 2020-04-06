/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/

#include "MojaveGLDefines.h"
#include "M_math.h"

/**************************************************************
* Constants
**************************************************************/

static const double
two54   =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
twom54  =  5.55111512312578270212e-17, /* 0x3C900000, 0x00000000 */
hugenum   = 1.0e+300,
tiny   = 1.0e-300;

/**************************************************************
* Internal Functions
**************************************************************/

double M_scalbn (double x, int n)
{
	int  k,hx,lx;
	hx = __HI(x);
	lx = __LO(x);
        k = (hx&0x7ff00000)>>20;		/* extract exponent */
        if (k==0) {				/* 0 or subnormal x */
            if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
	    x *= two54; 
	    hx = __HI(x);
	    k = ((hx&0x7ff00000)>>20) - 54; 
            if (n< -50000) return tiny*x; 	/*underflow*/
	    }
        if (k==0x7ff) return x+x;		/* NaN or Inf */
        k = k+n; 
        if (k >  0x7fe) return hugenum*M_copysign(hugenum,x); /* overflow  */
        if (k > 0) 				/* normal result */
	    {__HI(x) = (hx&0x800fffff)|(k<<20); return x;}
        if (k <= -54)
            if (n > 50000) 	/* in case integer overflow in n+k */
		return hugenum*M_copysign(hugenum,x);	/*overflow*/
	    else return tiny*M_copysign(tiny,x); 	/*underflow*/
        k += 54;				/* subnormal result */
        __HI(x) = (hx&0x800fffff)|(k<<20);
        return x*twom54;
}
