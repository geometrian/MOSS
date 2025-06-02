#pragma once

#include "__arch.h"



#define MOSS_ENV_USERLAND     0
#define MOSS_ENV_FREESTANDING 1
#define MOSS_ENV_THEVOID      2
#define MOSS_ENV_EMBEDWIN     3

#ifdef MOSS_ENV
	#if MOSS_ENV<MOSS_ENV_USERLAND || MOSS_ENV>MOSS_ENV_EMBEDWIN
		#error "Invalid environment!"
	#endif
#else
	#define MOSS_ENV MOSS_ENV_USERLAND
#endif



#ifdef __MOSS_CPP__
extern "C"
{
#endif

#if   MOSS_ENV == MOSS_ENV_USERLAND

	#error

#elif MOSS_ENV == MOSS_ENV_FREESTANDING

	__attribute__(( format(printf,1,2), noreturn ))
	void __moss_kpanic   ( char const* fmt_cstr, ...          );
	__attribute__(( format(printf,1,0), noreturn ))
	void __moss_kpanic_va( char const* fmt_cstr, va_list args );

	int __moss_putc_stdout( int ch );
	int __moss_putc_stderr( int ch );

	MOSS_ND int __moss_getc_stdin( void );
	MOSS_ND int __moss_getch( void );

#elif MOSS_ENV == MOSS_ENV_THEVOID

	MOSS_INLINE int __moss_putc_stdout( int ch ) { return ch; }
	MOSS_INLINE int __moss_putc_stderr( int ch ) { return ch; }

	MOSS_ND_INLINE int __moss_getc_stdin( void ) { return -1; /*EOF*/; }
	MOSS_ND_INLINE int __moss_getch( void ) { return -1; /*EOF*/; }

#elif MOSS_ENV == MOSS_ENV_EMBEDWIN

	int __moss_putc_stdout( int ch );
	int __moss_putc_stderr( int ch );

	MOSS_ND int __moss_getc_stdin( void );
	MOSS_ND int __moss_getch( void );

	MOSS_ND float  __moss_gt_expf( float  val );
	MOSS_ND double __moss_gt_expd( double val );
	MOSS_ND float  __moss_gt_logf( float val );

#else

	#error

#endif

#ifdef __MOSS_CPP__
}
#endif
