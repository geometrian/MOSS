#pragma once

/*
https://en.cppreference.com/w/c/string/wide
INCOMPLETE
*/

#include "__env.h"



#ifdef __MOSS_CPP__
namespace std
{
	extern "C"
	{
#endif



#include "__size_t.h"

/* Standard doesn't say where this awful type goes, but here is fine */
#if MOSS_ENV == MOSS_ENV_EMBEDWIN
	//typedef short wchar_t;
	typedef unsigned short wchar_t;
#else
	typedef int wchar_t;
#endif

#if MOSS_ENV==MOSS_ENV_FREESTANDING || MOSS_ENV==MOSS_ENV_EMBEDWIN
size_t wcslen( wchar_t const* str );
#endif



#ifdef __MOSS_CPP__
	}
}
#endif
