#pragma once

/*
https://en.cppreference.com/w/c/types/integer
*/

/* #include "__arch.h" */
#include "assert.h"



#ifdef __MOSS_CPP__
namespace std
{
	extern "C"
	{
#endif



typedef unsigned char      uint8_t ;
typedef   signed char       int8_t ;
typedef unsigned short     uint16_t;
typedef   signed short      int16_t;
typedef unsigned int       uint32_t;
typedef   signed int        int32_t;
typedef unsigned long long uint64_t;
typedef   signed long long  int64_t;
static_assert( sizeof(uint8_t )==1 && sizeof(int8_t )==1, "Invalid `uint8_t` size!"  );
static_assert( sizeof(uint16_t)==2 && sizeof(int16_t)==2, "Invalid `uint16_t` size!" );
static_assert( sizeof(uint32_t)==4 && sizeof(int32_t)==4, "Invalid `uint32_t` size!" );
static_assert( sizeof(uint64_t)==8 && sizeof(int64_t)==8, "Invalid `uint64_t` size!" );



#ifndef MOSS_LEAN

	typedef uint8_t  uint_fast8_t ;
	typedef  int8_t   int_fast8_t ;
	typedef uint32_t uint_fast16_t;
	typedef  int32_t  int_fast16_t;
	typedef uint32_t uint_fast32_t;
	typedef  int32_t  int_fast32_t;
	typedef uint64_t uint_fast64_t;
	typedef  int64_t  int_fast64_t;

	typedef uint8_t  uint_least8_t ;
	typedef  int8_t   int_least8_t ;
	typedef uint16_t uint_least16_t;
	typedef  int16_t  int_least16_t;
	typedef uint32_t uint_least32_t;
	typedef  int32_t  int_least32_t;
	typedef uint64_t uint_least64_t;
	typedef  int64_t  int_least64_t;

	typedef uint64_t uintmax_t;
	typedef  int64_t  intmax_t;

#endif

#if   defined __MOSS_32__
	typedef uint32_t uintptr_t;
	#ifndef MOSS_LEAN
	typedef  int32_t  intptr_t;
	#endif
#elif defined __MOSS_64__
	typedef uint64_t uintptr_t;
	#ifndef MOSS_LEAN
	typedef  int64_t  intptr_t;
	#endif
#else
	#error
#endif
static_assert( sizeof(uintptr_t)==sizeof(void*), "Invalid `uintptr_t` size!" );

#ifndef MOSS_LEAN

#if __MOSS_C__ >= 2023

	#define UINT8_WIDTH   8
	#define  INT8_WIDTH   8
	#define UINT16_WIDTH 16
	#define  INT16_WIDTH 16
	#define UINT32_WIDTH 32
	#define  INT32_WIDTH 32
	#define UINT64_WIDTH 64
	#define  INT64_WIDTH 64

	#define UINT_FAST8_WIDTH  ( 8*sizeof(uint_fast8_t ) )
	#define  INT_FAST8_WIDTH  ( 8*sizeof( int_fast8_t ) )
	#define UINT_FAST16_WIDTH ( 8*sizeof(uint_fast16_t) )
	#define  INT_FAST16_WIDTH ( 8*sizeof( int_fast16_t) )
	#define UINT_FAST32_WIDTH ( 8*sizeof(uint_fast32_t) )
	#define  INT_FAST32_WIDTH ( 8*sizeof( int_fast32_t) )
	#define UINT_FAST64_WIDTH ( 8*sizeof(uint_fast64_t) )
	#define  INT_FAST64_WIDTH ( 8*sizeof( int_fast64_t) )

	#define UINT_LEAST8_WIDTH  ( 8*sizeof(uint_least8_t ) )
	#define  INT_LEAST8_WIDTH  ( 8*sizeof( int_least8_t ) )
	#define UINT_LEAST16_WIDTH ( 8*sizeof(uint_least16_t) )
	#define  INT_LEAST16_WIDTH ( 8*sizeof( int_least16_t) )
	#define UINT_LEAST32_WIDTH ( 8*sizeof(uint_least32_t) )
	#define  INT_LEAST32_WIDTH ( 8*sizeof( int_least32_t) )
	#define UINT_LEAST64_WIDTH ( 8*sizeof(uint_least64_t) )
	#define  INT_LEAST64_WIDTH ( 8*sizeof( int_least64_t) )

	#define INTPTR_WIDTH ( 8*sizeof(intptr_t) )
	#define INTMAX_WIDTH ( 8*sizeof(intmax_t) )

#endif

#define INT8_MIN   ((int8_t )0x80                )
#define INT16_MIN  ((int16_t)0x8000              )
#define INT32_MIN  ((int32_t)0x80000000          )
#define INT64_MIN  ((int64_t)0x8000000000000000ll)
#if   defined __MOSS_32__
	#define INTPTR_MIN INT32_MIN
#elif defined __MOSS_64__
	#define INTPTR_MIN INT64_MIN
#else
	#error
#endif
#define INTMAX_MIN INT64_MIN
#define INT_FAST8_MIN   INT8_MIN
#define INT_FAST16_MIN  INT32_MIN
#define INT_FAST32_MIN  INT32_MIN
#define INT_FAST64_MIN  INT64_MIN
#define INT_LEAST8_MIN  INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT8_MAX   ( (int8_t  )~INT8_MIN   )
#define INT16_MAX  ( (int16_t )~INT16_MIN  )
#define INT32_MAX  ( (int32_t )~INT32_MIN  )
#define INT64_MAX  ( (int64_t )~INT64_MIN  )
#define INTPTR_MAX ( (intptr_t)~INTPTR_MIN )
#define INTMAX_MAX ( (intmax_t)~INTMAX_MIN )
#define INT_FAST8_MAX   INT8_MAX
#define INT_FAST16_MAX  INT32_MAX
#define INT_FAST32_MAX  INT32_MAX
#define INT_FAST64_MAX  INT64_MAX
#define INT_LEAST8_MAX  INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

#define UINT8_MAX   ( int8_t  ((int8_t  )~(int8_t  )0) >> 1 )
#define UINT16_MAX  ( int16_t ((int16_t )~(int16_t )0) >> 1 )
#define UINT32_MAX  ( int32_t ((int32_t )~(int32_t )0) >> 1 )
#define UINT64_MAX  ( int64_t ((int64_t )~(int64_t )0) >> 1 )
#define UINTPTR_MAX ( intptr_t((intptr_t)~(intptr_t)0) >> 1 )
#define UINTMAX_MAX ( intmax_t((intmax_t)~(intmax_t)0) >> 1 )
#define UINT_FAST8_MAX   UINT8_MAX
#define UINT_FAST16_MAX  UINT32_MAX
#define UINT_FAST32_MAX  UINT32_MAX
#define UINT_FAST64_MAX  UINT64_MAX
#define UINT_LEAST8_MAX  UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define UINT8_C(VAL)   ( (uint_least8_t )(val) )
#define  INT8_C(VAL)   ( ( int_least8_t )(val) )
#define UINT16_C(VAL)  ( (uint_least16_t)(val) )
#define  INT16_C(VAL)  ( ( int_least16_t)(val) )
#define UINT32_C(VAL)  ( (uint_least32_t)(val) )
#define  INT32_C(VAL)  ( ( int_least32_t)(val) )
#define UINT64_C(VAL)  ( (uint_least64_t)(val) )
#define  INT64_C(VAL)  ( ( int_least64_t)(val) )
#define UINTMAX_C(VAL) ( (uintmax_t     )(val) )
#define  INTMAX_C(VAL) ( ( intmax_t     )(val) )

#define MOSS_INT_MIN INT32_MIN
#define MOSS_INT_MAX INT32_MAX

#endif



#ifdef __MOSS_CPP__
	}
}
#endif
