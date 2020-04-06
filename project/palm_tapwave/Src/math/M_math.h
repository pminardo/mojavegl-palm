/**************************************************************
* Copyright (c) 2006 PDA Performance, Inc.
* All rights reserved.
**************************************************************/
#ifndef __MT_MATH_H_
#define __MT_MATH_H_

// MojaveGLDefines.h MUST be included before this file; otherwise,
// the wrong HI and LO macros will be used.

#ifdef __LITTLE_ENDIAN
#define __HI(x) *(1+(int*)&x)
#define __LO(x) *(int*)&x
#define __HIp(x) *(1+(int*)x)
#define __LOp(x) *(int*)x
#else
#define __HI(x) *(int*)&x
#define __LO(x) *(1+(int*)&x)
#define __HIp(x) *(int*)x
#define __LOp(x) *(1+(int*)x)
#endif

/**************************************************************
* MojaveGL Math Functions
**************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

double M__kernel_cos(double x, double y);
double M__kernel_sin(double x, double y, int iy);
int M__kernel_rem_pio2(double *x, double *y, int e0, int nx, int prec, const int *ipio2);

double M_cos(double x);
double M_sin(double x);
double M_sqrt(double x);

double M_fabs(double x);
double M_scalbn (double x, int n);
double M_copysign(double x, double y);
double M_floor(double x);

int M__ieee754_rem_pio2(double x, double *y);
double M__ieee754_sqrt(double x);

#ifdef __cplusplus
}
#endif


#endif // __MT_MATH_H_