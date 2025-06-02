#pragma once

/*
https://en.cppreference.com/w/c/error
COMPLETE
*/

#include "__arch.h"



#ifdef NDEBUG

	#define assert( PASS_COND ) ((void)0)

#else

	#define assert( PASS_COND )\
		do\
		{\
			if (!(PASS_COND)) __moss_assert_fail1(\
				__FILE__,__LINE__, __func__, #PASS_COND\
			);\
		}\
		while (0)

#endif

#if defined __MOSS_C__ && __MOSS_C__>=2011 && __MOSS_C__<2023
	#ifndef __MOSS_CLANG__
		#define static_assert _Static_assert
	#endif
#endif
