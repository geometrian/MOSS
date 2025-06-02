#pragma once

/*
https://en.cppreference.com/w/c/variadic
COMPLETE
*/

#include "__arch.h"



#if   defined __MOSS_CLANG__ || defined __MOSS_GCC__

	typedef __builtin_va_list va_list;
	#define va_start( args, param_before ) (__builtin_va_start( args, param_before ))
	#define va_end( args )                 (__builtin_va_end( args ))
	#define va_arg( args, type )           (__builtin_va_arg( args, type ))

	#if ( defined __MOSS_C__ && __MOSS_C__>=1999 ) || ( defined __MOSS_CPP__ && __MOSS_CPP__>=2011 )
		#define va_copy( dst, src ) (__builtin_va_copy( dst, src ))
	#endif

#elif defined __MOSS_MSVC__

	typedef char* va_list;

	/* https://stackoverflow.com/a/32293441/688624 */
	#define va_start( args, param_before ) (  (void) ( (args) = (char*)(&(param_before)+1)   )     )
	#define va_end( args )                 (  (void) ( (args) = 0                            )     )
	#define va_arg( args, type )           ( ((type*)( (args) = ((args)+sizeof(type)) )      )[-1] )

	#if ( defined __MOSS_C__ && __MOSS_C__>=1999 ) || ( defined __MOSS_CPP__ && __MOSS_CPP__>=2011 )
		#define va_copy( dst, src ) ( (dst) = (src) )
	#endif

#else

	#error

#endif
