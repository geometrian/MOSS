#pragma once

/*
https://en.cppreference.com/w/cpp/header/cstring
COMPLETE except for `strxfrm(⋯)`, `strcoll(⋯)`, `strtok(⋯)`, and `strerror(⋯)`
*/

#include "__arch.h"



#ifdef __MOSS_CPP__
namespace std
{
	extern "C"
	{
#endif



#ifndef NULL
	#define NULL 0
#endif

#include "__size_t.h"

MOSS_ND int __moss_str_contains( char const* str, char nonnull_ch );

MOSS_ND int __moss_str_startswith( char const* str, char const* prefix );

/* Reverses bytes in range [`start`,`end`). */
void __moss_reverse( void* start, void* end );

/* Copies characters from `src` to `dst` until `src` runs out. */
char* strcpy ( char*__restrict dst,char const*__restrict src               );
/* Copies exactly `count` characters from `src` to `dst` (taking '\0' if `src` runs out). */
char* strncpy( char*__restrict dst,char const*__restrict src, size_t count );

/* Writes `src` at the end of `dst`*/
char* strcat ( char*__restrict dst, char const*__restrict src                   );
/* Writes up to `max_count` chars from `src` at the end of `dst` (note another +1 for '\0'!). */
char* strncat( char*__restrict dst, char const*__restrict src, size_t max_count );

/* size_t strxfrm( void*__restrict dst,void const*__restrict src, size_t count ); */

/* Gets the length of the string (not including '\0') */
MOSS_ND size_t strlen( char const* str );

/* Compares strings character by character; the first difference is returned as `lhs[k]-rhs[k]`. */
MOSS_ND int strcmp ( char const* lhs,char const* rhs                   );
/* Like `strcmp(⋯)`, but compares at most `max_count` characters */
MOSS_ND int strncmp( char const* lhs,char const* rhs, size_t max_count );

/* MOSS_ND_INLINE int strcoll( char const* lhs, char const* rhs ); */

/* Note removes `const`; evil; needed for C */
MOSS_ND char* __moss_strchr ( char const* str, int ch );
MOSS_ND char* __moss_strrchr( char const* str, int ch );
#ifdef __MOSS_C__ /* C++ overloads are different, in <cstring> */
/* Finds the first occurrence of `ch` in `str`; `NULL` if not present */
MOSS_ND_INLINE char* strchr ( char const* str, int ch ) { return __moss_strchr ( str, ch ); }
/* Finds the last  occurrence of `ch` in `str`; `NULL` if not present */
MOSS_ND_INLINE char* strrchr( char const* str, int ch ) { return __moss_strrchr( str, ch ); }
#endif

/* Returns the length of the prefix of `test_str` that only uses chars       from `dict_str`. */
MOSS_ND size_t strspn ( char const* test_str, char const* dict_str );
/* Returns the length of the prefix of `test_str` that only uses chars *not* from `dict_str`. */
MOSS_ND size_t strcspn( char const* test_str, char const* dict_str );

/* Note removes `const`; evil; needed for C */
MOSS_ND char* __moss_strpbrk( char const* find_str, char const* dict_str );
#ifdef __MOSS_C__ /* C++ overloads are different, in <cstring> */
/* Searches `find_str` for any char in `dict_str`, returning the first location (`NULL` if none). */
MOSS_ND_INLINE char* strpbrk( char const* find_str, char const* dict_str )
{
	return __moss_strpbrk( find_str, dict_str );
}
#endif

/* Note removes `const`; evil; needed for C */
MOSS_ND char* __moss_strstr( char const* in_str, char const* search_str );
#ifdef __MOSS_C__ /* C++ overloads are different, in <cstring> */
/* Searches for `search_str` in `in_str`, returning the first location (`NULL` if none). */
MOSS_ND_INLINE char* strstr( char const* in_str, char const* search_str )
{
	return __moss_strstr( in_str, search_str );
}
#endif

/*
Parsing function.  Give it a buffer `str_buf` on the first invocation and `NULL` afterward.  Will
return null-terminated strings ('tokens') broken at characters in `delims` and finally `NULL` when
it runs out.

It does this by dropping '\0's into `str_buf`.  It also has to store state, so note this is *not*
thread-safe!
*/
MOSS_ND char* strtok( char* str_buf, char const* delims );

/* Note removes `const`; evil; needed for C */
MOSS_ND void* __moss_memchr( void const* data, int ch, size_t size );
#ifdef __MOSS_C__ /* C++ overloads are different, in <cstring> */
/*
Searches buffer `data` (of size `size`) for `ch` (interpreted as uint8), returning the first
location (`NULL` if none).
*/
MOSS_ND_INLINE void* memchr( void const* data, int ch, size_t size )
{
	return __moss_memchr( data, ch, size );
}
#endif

MOSS_ND int memcmp( void const* lhs,void const* rhs, size_t size );

void* memset( void* data, int byte, size_t size );

void* memcpy ( void*__restrict dst,void const*__restrict src, size_t size );
void* memmove( void*           dst,void const*           src, size_t size );

/* char* strerror( int errnum ); */



#ifdef __MOSS_CPP__
	}
}
#endif



extern char const*const __moss_xchrs; /* "0123456789abcdef" */
extern char const*const __moss_Xchrs; /* "0123456789ABCDEF" */

extern char const*const __moss_si_pos; /* "kMGTPEZYRQ" */
