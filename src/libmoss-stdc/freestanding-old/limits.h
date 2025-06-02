#pragma once

/*
https://en.cppreference.com/w/c/types/limits
COMPLETE except for `BITINT_MAXWIDTH`
*/

#include "__arch.h"



#if defined __MOSS_C__ && __MOSS_C__>=2023
	#define BOOL_WIDTH (8*sizeof(_Bool))
#endif

#define CHAR_BIT 8

#define MB_LEN_MAX 2

#if defined __MOSS_C__ && __MOSS_C__>=2023
	#define CHAR_WIDTH CHAR_BIT
#endif

#define CHAR_MIN ((char)(-128))
#define CHAR_MAX ((char)( 127))

#if defined __MOSS_C__ && __MOSS_C__>=2023
	#define SCHAR_WIDTH (8*sizeof(signed char))
	#define SHRT_WIDTH  (8*sizeof(short      ))
	#define INT_WIDTH   (8*sizeof(int        ))
	#define LONG_WIDTH  (8*sizeof(long       ))
	#define LLONG_WIDTH (8*sizeof(long long  ))
#endif

#if defined __MOSS_C__ && __MOSS_C__>=2023
	#define UCHAR_WIDTH  (8*sizeof(unsigned char     ))
	#define USHRT_WIDTH  (8*sizeof(unsigned short    ))
	#define UINT_WIDTH   (8*sizeof(unsigned int      ))
	#define ULONG_WIDTH  (8*sizeof(unsigned long     ))
	#define ULLONG_WIDTH (8*sizeof(unsigned long long))
#endif

#define SCHAR_MIN ((signed char)(-128))
#define SHRT_MIN  ((short      )( 1LL << (8*sizeof(short    )-1) ))
#define INT_MIN   ((int        )( 1LL << (8*sizeof(int      )-1) ))
#define LONG_MIN  ((long       )( 1LL << (8*sizeof(long     )-1) ))
#if ( defined __MOSS_C__ && __MOSS_C__>=1999 ) || ( defined __MOSS_CPP__ && __MOSS_CPP__>=2011 )
#define LLONG_MIN ((long long  )( 1LL << (8*sizeof(long long)-1) ))
#endif

#define SCHAR_MAX ((signed char)(127       ))
#define SHRT_MAX  ((short      )(~SHRT_MIN ))
#define INT_MAX   ((int        )(~INT_MIN  ))
#define LONG_MAX  ((long       )(~LONG_MIN ))
#if ( defined __MOSS_C__ && __MOSS_C__>=1999 ) || ( defined __MOSS_CPP__ && __MOSS_CPP__>=2011 )
#define LLONG_MAX ((long long  )(~LLONG_MIN))
#endif

#define UCHAR_MAX  ((unsigned char     )( ~(unsigned char     )0 ))
#define USHRT_MAX  ((unsigned short    )( ~(unsigned short    )0 ))
#define UINT_MAX   ((unsigned int      )( ~(unsigned int      )0 ))
#define ULONG_MAX  ((unsigned long     )( ~(unsigned long     )0 ))
#if ( defined __MOSS_C__ && __MOSS_C__>=1999 ) || ( defined __MOSS_CPP__ && __MOSS_CPP__>=2011 )
#define ULLONG_MAX ((unsigned long long)( ~(unsigned long long)0 ))
#endif

#if defined __MOSS_C__ && __MOSS_C__>=2023
	/* BITINT_MAXWIDTH */
#endif
