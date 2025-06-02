#include "wchar.h"

#include "assert.h"



#if MOSS_ENV == MOSS_ENV_EMBEDWIN
	static_assert( sizeof(wchar_t)==2, "Invalid `wchar_t` size!" );
#else
	static_assert( sizeof(wchar_t)==4, "Invalid `wchar_t` size!" );
#endif



#if MOSS_ENV==MOSS_ENV_FREESTANDING || MOSS_ENV==MOSS_ENV_EMBEDWIN
size_t wcslen( wchar_t const* str )
{
	wchar_t const* str_orig = str;
	while ( *str != '\0' ) ++str;
	return (size_t)( str - str_orig );
}
#endif
