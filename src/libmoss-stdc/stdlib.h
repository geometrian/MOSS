#pragma once

/*
https://en.cppreference.com/w/cpp/header/cstdlib
*/



#include "__size_t.h"



#ifdef __MOSS_CPP__
namespace std
{
	extern "C"
	{
#endif



__attribute__((noreturn))
void abort( void );

__attribute__((noreturn))
void exit( int status );

int atexit( void (*fn)(void) );



void __moss_memory_dump( void );

/* Returns result aligned to at least `alignof(max_align_t)` */
void* malloc( size_t size );
/* Returns result aligned to at least `align` */
void* aligned_alloc( size_t align, size_t size );

void free( void* ptr );



int       atoi ( char const* str );
long      atol ( char const* str );
long long atoll( char const* str );



#ifdef __MOSS_CPP__
	}
}
#endif
