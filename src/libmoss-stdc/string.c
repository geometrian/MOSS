#include "string.h"

#include "assert.h"



MOSS_ND int __moss_str_contains( char const* str, char nonnull_ch )
{
	LOOP:
		if ( *str != '\0' )
		{
			if ( *str == nonnull_ch ) return 1;
			++str;
			goto LOOP;
		}
	return 0;
}

MOSS_ND int __moss_str_startswith( char const* str, char const* prefix )
{
	LOOP:
		if ( *prefix == '\0' ) return 1;
		if ( *str == *prefix )
		{
			++str; ++prefix;
			goto LOOP;
		}
	return 0;
}

void __moss_reverse( void* start, void* end )
{
	static_assert( sizeof(char)==1, "Invalid `char` size!" );

	size_t count = (size_t)( (char*)end - (char*)start );
	for ( size_t i=0; i<(count>>1); ++i )
	{
		char* ptr0 = ((char*)start) +            i  ;
		char* ptr1 = ((char*)start) + ((count-1)-i) ;

		char tmp = *ptr0;
		*ptr0 = *ptr1;
		*ptr1 = tmp;
	}
}

char* strcpy ( char*__restrict dst,char const*__restrict src               )
{
	char* dst_orig = dst;
	LOOP:
		*dst = *src;
		if ( *src != '\0' ) { ++dst; ++src; goto LOOP; }
	return dst_orig;
}
char* strncpy( char*__restrict dst,char const*__restrict src, size_t count )
{
	assert( dst!=NULL && src!=NULL );

	char* dst_orig = dst;
	if ( count > 0 )
	{
		LOOP1:
		*dst=*src; --count;
		if ( count > 0 )
		{
			++dst;
			if ( *src != '\0' )
			{
				++src;
				goto LOOP1;
			}

			LOOP2:
			*dst='\0'; --count;
			if ( count > 0 )
			{
				++dst;
				goto LOOP2;
			}
		}
	}
	return dst_orig;
}

char* strcat ( char*__restrict dst, char const*__restrict src                   )
{
	char* dst_orig = dst;

	dst += strlen( dst );

	LOOP:
		*dst = *src;
		if ( *src != '\0' )
		{
			++dst; ++src;
			goto LOOP;
		}

	return dst_orig;
}
char* strncat( char*__restrict dst, char const*__restrict src, size_t max_count )
{
	if ( max_count == 0 ) return dst;
	char* dst_orig = dst;

	while ( *dst != '\0' ) ++dst;

	LOOP:
		*dst = *src;
		if ( *src!='\0' && --max_count>0 )
		{
			++dst; ++src;
			goto LOOP;
		}

	return dst_orig;
}

/* Note `__builtin_strlen(⋯)` causes infinite recursion */
MOSS_ND size_t strlen( char const* str )
{
	char const* str_orig = str;
	while ( *str != '\0' ) ++str;
	return (size_t)( str - str_orig );
}

/* Note `__builtin_strcmp(⋯)` and `__builtin_strncmp(⋯)` probably cause infinite recursion */
MOSS_ND int strcmp ( char const* lhs,char const* rhs                   )
{
	assert( lhs!=NULL && rhs!=NULL );

	unsigned char const *ptr0=(unsigned char const*)lhs, *ptr1=(unsigned char const*)rhs;
	LOOP:
		if ( *ptr0 != *ptr1 ) return (int)(*ptr0)-(int)(*ptr1);
		if ( *ptr0 != '\0' )
		{
			++ptr0; ++ptr1;
			goto LOOP;
		}
	return 0;
}
MOSS_ND int strncmp( char const* lhs,char const* rhs, size_t max_count )
{
	assert( lhs!=NULL && rhs!=NULL );

	unsigned char const *ptr0=(unsigned char const*)lhs, *ptr1=(unsigned char const*)rhs;
	LOOP:
		if ( max_count > 0 )
		{
			if ( *ptr0 != *ptr1 ) return (int)(*ptr0)-(int)(*ptr1);
			if ( *ptr0 != '\0' )
			{
				--max_count;
				++ptr0; ++ptr1;
				goto LOOP;
			}
		}
	return 0;
}

/* Note `__builtin_strchr(⋯)` (the only one available) probably causes infinite recursion */
MOSS_ND char* __moss_strchr ( char const* str, int ch )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wcast-qual"
	#endif

	assert( str != NULL );
	LOOP:
		if ( *str == ch ) return (char*)str; /* note '\0' can be detected, and evil cast */
		if ( *str != '\0' )
		{
			++str;
			goto LOOP;
		}
	return NULL;

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}
MOSS_ND char* __moss_strrchr( char const* str, int ch )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wcast-qual"
	#endif

	assert( str != NULL );
	char const* last = NULL;
	LOOP:
		if ( *str == ch ) last=str; /* note '\0' can be detected */
		if ( *str != '\0' )
		{
			++str;
			goto LOOP;
		}
	return (char*)last; /* evil */

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}

/* TODO: can optimize for the special cases e.g. when length of `dict_str` is 0 or 1 */
MOSS_ND size_t strspn ( char const* test_str, char const* dict_str )
{
	char const* test_str_orig = test_str;
	while ( *test_str != '\0' )
	{
		if ( __moss_str_contains( dict_str, *test_str )) ++test_str;
		else break;
	}
	return (size_t)( test_str - test_str_orig );
}
MOSS_ND size_t strcspn( char const* test_str, char const* dict_str )
{
	char const* test_str_orig = test_str;
	while ( *test_str != '\0' )
	{
		if (!__moss_str_contains( dict_str, *test_str )) ++test_str;
		else break;
	}
	return (size_t)( test_str - test_str_orig );
}

MOSS_ND char* __moss_strpbrk( char const* find_str, char const* dict_str )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wcast-qual"
	#endif

	LOOP:
		if ( *find_str != '\0' )
		{
			if ( __moss_str_contains( dict_str, *find_str ) ) return (char*)find_str; /* evil */
			++find_str;
			goto LOOP;
		}
	return NULL;

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}

MOSS_ND char* __moss_strstr( char const* in_str, char const* search_str )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wcast-qual"
	#endif

	if ( *search_str == '\0' ) return (char*)in_str; /* optimization */

	while ( *in_str != '\0' )
	{
		if ( __moss_str_startswith( in_str, search_str ) ) return (char*)in_str;
		++in_str;
	}

	return NULL;

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}

MOSS_ND char* strtok( char* str_buf, char const* delims )
{
	static char* buffer;
	if ( str_buf != NULL ) buffer=str_buf;

	//Skip any delimiters
	buffer += strspn( buffer, delims );
	if ( *buffer=='\0' ) return NULL;

	//Token: skip ahead until the next delimiter and null-terminate there.
	char* tok_begin = buffer;
	buffer += strcspn( buffer, delims );
	if ( *buffer != '\0' ) *(buffer++)='\0';

	//Then return from the start of the token
	return tok_begin;
}

/* Note `__builtin_memchr(⋯)` probably causes infinite recursion */
MOSS_ND void* __moss_memchr( void const* data, int ch, size_t size )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wcast-qual"
	#endif

	unsigned char const* ptr = (unsigned char const*)data;
	unsigned char val = (unsigned char)ch;
	if ( size > 0 )
	{
		LOOP:
		if ( *ptr == val ) return (void*)ptr; /* evil */
		if ( --size > 0) { ++ptr; goto LOOP; }
	}
	return NULL;

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}

/* Note `__builtin_memcmp(⋯)` probably causes infinite recursion */
MOSS_ND int memcmp( void const* lhs,void const* rhs, size_t size )
{
	if ( size == 0 ) goto END;

	unsigned char const *ptr0=(unsigned char const*)lhs, *ptr1=(unsigned char const*)rhs;

	LOOP:
		if ( *ptr0 != *ptr1 ) return (int)(*ptr0)-(int)(*ptr1);
		if ( --size > 0) { ++ptr0; ++ptr1; goto LOOP; }

	END:
	return 0;
}

/* Note `__builtin_memset(⋯)` probably causes infinite recursion */
void* memset( void* data, int byte, size_t size )
{
	if ( size == 0 ) goto END;

	unsigned char* ptr = (unsigned char*)data;
	unsigned char val = (unsigned char)byte;

	LOOP:
		*ptr = val;
		if ( --size > 0 ) { ++ptr; goto LOOP; }

	END:
	return data;
}

/* Note `__builtin_memcpy(⋯)` probably causes infinite recursion */
void* memcpy ( void*__restrict dst,void const*__restrict src, size_t size )
{
	if ( size == 0 ) goto END;

	unsigned char      * dstu8 = (unsigned char      *)dst;
	unsigned char const* srcu8 = (unsigned char const*)src;

	// If intentional, use `memmove(⋯)` instead.
	__moss_assert( dstu8+size<=srcu8 || srcu8+size<=dstu8,
		"Invalid call to `memcpy`; memory regions overlap!"
	);

	LOOP:
		*dstu8 = *srcu8;
		if ( --size > 0 ) { ++dstu8; ++srcu8; goto LOOP; }

	END:
	return dst;
}
/* Note `__builtin_memmove(⋯)` probably causes infinite recursion */
void* memmove( void*           dst,void const*           src, size_t size )
{
	/*
	Same as above, but `dst` and `src` aren't `__restrict` and are allowed to overlap, so maybe it
	optimizes worse.
	*/

	if ( size == 0 ) goto END;

	unsigned char      * dstu8 = (unsigned char      *)dst;
	unsigned char const* srcu8 = (unsigned char const*)src;

	if      ( dstu8 < srcu8 ) // Moving backward
	{
		LOOP_1:
			*dstu8 = *srcu8;
			if ( --size > 0 ) { ++dstu8; ++srcu8; goto LOOP_1; }
	}
	else if ( srcu8 < dstu8 ) // Moving forward
	{
		srcu8 += size - 1;
		dstu8 += size - 1;
		LOOP_2:
			*dstu8 = *srcu8;
			if ( --size > 0 ) { --dstu8; --srcu8; goto LOOP_2; }
	}

	END:
	return dst;
}



char const*const __moss_xchrs = "0123456789abcdef";
char const*const __moss_Xchrs = "0123456789ABCDEF";

char const*const __moss_si_pos = "kMGTPEZYRQ";
