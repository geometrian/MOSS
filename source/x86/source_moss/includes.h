#pragma once

//Note cannot be e.g. <cstddef>; cross-compilers evidently do not support that.
#include <stddef.h>
#include <stdint.h>

//TODO: a shorter file called "includes.h" should be included in _every_ header file so
//that global defines can be found.  The other headers in this file can be included directly instead.


#define MOSS_DEBUG

#ifdef MOSS_DEBUG
	#define MOSS_DEBUG_BOCHSBREAK __asm__ __volatile__("xchg  %bx, %bx")
#else
	#define MOSS_DEBUG_BOCHSBREAK
#endif

#if !defined(__cplusplus)
	#error "C++ should be used!"
#endif
#if defined(__linux__)
	#error "You are not using a cross-compiler!"
#endif

namespace MOSS {
	#ifdef MOSS_DEBUG
		void _assert(bool condition, const char* filename,int line, const char* failure_message,...);
		//Double hash removes trailing comma when arguments are empty.  See http://en.wikipedia.org/wiki/Variadic_macro#The_impossible_example
		#define ASSERT(CONDITION,FAILURE_MESSAGE,...) ::MOSS::_assert(CONDITION, __FILE__,__LINE__, FAILURE_MESSAGE,##__VA_ARGS__)
	#else
		#define ASSERT(CONDITION,FAILURE_MESSAGE,...)
	#endif
}