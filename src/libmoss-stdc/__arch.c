#include "__arch.h"

#include "__env.h"
#include "stdio.h"



#ifndef NDEBUG /* debug */



MOSS_NORET_ATTRIB void __moss_assert_fail1   (
	char const* file,int line, char const* fnname, char const* cond
) {
	#if MOSS_ENV == MOSS_ENV_FREESTANDING
		__moss_kpanic(
			"Assertion failure:\n"
			"`%s`\n"
			"At %s:%d in `%s`.",
			cond, file,line, fnname
		);
	#else
		fprintf( stderr, "%s:%d in `%s`: assertion `%s` failed!\n", file,line, fnname, cond );

		#if   defined __MOSS_CLANG__ || defined __MOSS_GCC__
			__builtin_trap();
		#elif defined __MOSS_MSVC__
			__debugbreak();
		#endif

		//std::abort();
		//while (1) {}
	#endif
}

MOSS_NORET_ATTRIB void __moss_assert_fail2   (
	char const* file,int line, char const* fnname, char const* fmt_cstr,...
) {
	va_list args;
	va_start( args, fmt_cstr );
	__moss_assert_fail2_va( file,line, fnname, fmt_cstr,args );
	va_end( args );
}
MOSS_NORET_ATTRIB void __moss_assert_fail2_va(
	char const* file,int line, char const* fnname, char const* fmt_cstr,va_list args
) {
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wformat-nonliteral"
		#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
		#pragma clang diagnostic ignored "-Wformat-extra-args"
	#endif

	#if MOSS_ENV == MOSS_ENV_FREESTANDING
		__moss_kpanic(
			"Assertion failure at %s:%d in `%s`:\n%V\n",
			file,line, fnname, fmt_cstr,&args
		);
	#else
		fprintf( stderr,
			"Assertion failure at %s:%d in `%s`:\n%V\n",
			file,line, fnname, fmt_cstr,&args
		);
	#endif

	#if MOSS_ENV != MOSS_ENV_FREESTANDING
		#if   defined __MOSS_CLANG__ || defined __MOSS_GCC__
			__builtin_trap();
		#elif defined __MOSS_MSVC__
			__debugbreak();
		#endif

		//std::abort();
		//while (1) {}
	#endif

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}



// https://wiki.osdev.org/Stack_Smashing_Protector

#if MOSS_ENV == MOSS_ENV_FREESTANDING
	#ifdef __MOSS_32__
	unsigned int       __stack_chk_guard = 0xe2dee396        ;
	#else
	unsigned long long __stack_chk_guard = 0x595e9fbd94fda766;
	#endif
#else
	// TODO: generate securely at program startup
#endif

__attribute__((noreturn))
void __stack_chk_fail(void)
{
	#if MOSS_ENV == MOSS_ENV_FREESTANDING
		__moss_assert_false( "Stack smashing detected!" );
	#else
		// Abort ASAP; if this is an attack, other actions may leak information or play into
		// attacker's hands.
		abort();
	#endif
}



#endif
