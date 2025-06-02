#pragma once

/*
https://en.cppreference.com/w/c/types/limits#Limits_of_floating-point_types
COMPLETE except for `FLT_ROUNDS`
*/

#include "__arch.h"



#define FLT_RADIX 2

/* Number of digits guaranteed to be preserved in conversion float → text → float */
#if __MOSS_C__>=1999 || __MOSS_CPP__>=2011
	#define DECIMAL_DIG 17
#endif
#if __MOSS_C__>=2011 || __MOSS_CPP__>=2017
	#define  FLT_DECIMAL_DIG  9
	#define  DBL_DECIMAL_DIG 17
	#define LDBL_DECIMAL_DIG DBL_DECIMAL_DIG
#endif

/* Minimum positive normalized value */
#define  FLT_MIN 1.175494351e-38f
#define  DBL_MIN 2.2250738585072014e-308
#define LDBL_MIN DBL_MIN

/* Minimum positive value (is subnormal) */
#if __MOSS_C__>=2011 || __MOSS_CPP__>=2017
	#define  FLT_TRUE_MIN 1.401298464e-45f
	#define  DBL_TRUE_MIN 4.9406564584124654e-324
	#define LDBL_TRUE_MIN DBL_TRUE_MIN
#endif

/* Maximum non-infinite value */
#define  FLT_MAX 3.402823466e+38f
#define  DBL_MAX 1.7976931348623158e+308
#define LDBL_MAX DBL_MAX

/* Difference between 1 and the next representable value */
#define  FLT_EPSILON 1.192092896e-07f
#define  DBL_EPSILON 2.2204460492503131e-016
#define LDBL_EPSILON DBL_EPSILON

/* Number of digits guaranteed to be preserved in conversion text → float → text */
#define  FLT_DIG  6
#define  DBL_DIG 15
#define LDBL_DIG DBL_DIG

/* Number of bits in mantissa (including implicit leading 1) */
#define  FLT_MANT_DIG 24
#define  DBL_MANT_DIG 53
#define LDBL_MANT_DIG DBL_MANT_DIG

/* Minimum (negative) integer k such that 2^(k-1) is still normalized */
#define  FLT_MIN_EXP ( -125)
#define  DBL_MIN_EXP (-1021)
#define LDBL_MIN_EXP DBL_MIN_EXP

/* Minimum (negative) integer k such that 10^(k-1) is still normalized */
#define  FLT_MIN_10_EXP ( -37)
#define  DBL_MIN_10_EXP (-307)
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP

/* Maximum integer k such that 2^(k-1) is still finite */
#define  FLT_MAX_EXP  128
#define  DBL_MAX_EXP 1024
#define LDBL_MAX_EXP DBL_MAX_EXP

/* Maximum integer k such that 10^(k-1) is still finite */
#define  FLT_MAX_10_EXP  38
#define  DBL_MAX_10_EXP 308
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP

/* FLT_ROUNDS */

/* Precision works according to the types given */
#if defined __MOSS_C__ || __MOSS_CPP__>=2011
	#define FLT_EVAL_METHOD 0
#endif

/* Whether subnormal float-point is supported */
#if __MOSS_C__>=2011 || __MOSS_CPP__>=2017
	#define  FLT_HAS_SUBNORM 1
	#define  DBL_HAS_SUBNORM 1
	#define LDBL_HAS_SUBNORM DBL_HAS_SUBNORM
#endif
