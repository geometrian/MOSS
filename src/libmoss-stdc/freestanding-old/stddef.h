#pragma once

#include "__arch.h"

#include "__size_t.h"



#if   defined __MOSS_C__   && __MOSS_C__   >= 2023
	typedef typeof(nullptr) nullptr_t;
#elif defined __MOSS_CPP__ && __MOSS_CPP__ >= 2011
	namespace std
	{
		using nullptr_t = decltype(nullptr);
	}
#endif

#ifndef NULL
	#define NULL 0
#endif

#ifdef __MOSS_CPP__
	namespace std
	{
		using max_align_t = double;
	}

	#ifdef MOSS_LLVM_LIBCXX
	typedef double max_align_t;
	#endif
#else
	/* https://en.cppreference.com/w/c/types/max_align_t */
	typedef double max_align_t;
#endif

#define offsetof( type, member )\
	( (size_t)&( ((type*)0)->member ) )

#if   defined __MOSS_32__
	typedef int       ptrdiff_t;
#elif defined __MOSS_64__
	typedef long long ptrdiff_t;
#else
	#error
#endif
