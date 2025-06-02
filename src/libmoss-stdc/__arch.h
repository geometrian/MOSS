#pragma once



#ifdef __INTELLISENSE__
	#define __x86_64__
	/*#ifdef __cplusplus
		#undef __cplusplus
		#define __cplusplus 202002L
	#endif*/
	#define __MOSS_WITH_INTS__
	#define MOSS_ENV 3
	#undef _MSC_VER
	#define MOSS_NO_INTELLISENSE( CODE )
#else
	#define MOSS_NO_INTELLISENSE( CODE ) CODE
#endif



#if   defined __IA_32__
	#define __MOSS_32__
	#define __MOSS_X86__
#elif defined __x86_64__
	#define __MOSS_64__
	#define __MOSS_AMD64__
#else
	#error
#endif

#if   defined __cplusplus
	#if   __cplusplus >  202002L /* C++23 */
		#define __MOSS_CPP__ 2023
	#elif __cplusplus == 202002L /* C++20 */
		#define __MOSS_CPP__ 2020
	#elif __cplusplus == 201703L /* C++17 */
		#define __MOSS_CPP__ 2017
	#elif __cplusplus == 201402L /* C++14 */
		#define __MOSS_CPP__ 2014
	#elif __cplusplus == 201103L /* C++11 */
		#define __MOSS_CPP__ 2011
	#elif __cplusplus == 199711L /* C++98 or (C++98 and TR1) */
		#define __MOSS_CPP__ 1998
	#elif __cplusplus == 1       /* before */
		#error
	#else                        /* unknown */
		#error
	#endif

	#define MOSS_INLINE inline
	#define MOSS_CEXPR  constexpr
	#define MOSS_ND [[nodiscard]]
	#define MOSS_ND_INLINE [[nodiscard]] inline
	#define MOSS_ND_CEXPR  [[nodiscard]] constexpr
	#define MOSS_NOEXCEPT noexcept
#elif defined __STDC_VERSION__
	#if   __STDC_VERSION__ >  202000L /* C23 */
		#define __MOSS_C__ 2023
	#elif __STDC_VERSION__ == 202000L /* C20 */
		#define __MOSS_C__ 2020
	#elif __STDC_VERSION__ == 201710L /* C17 */
		#define __MOSS_C__ 2017
	#elif __STDC_VERSION__ == 201112L /* C11 */
		#define __MOSS_C__ 2011
	#elif __STDC_VERSION__ == 199901L /* C99 */
		#define __MOSS_C__ 1999
	#elif __STDC_VERSION__ == 199409L /* C89 amended 1994 */
		#define __MOSS_C__ 1989
	#else                             /* unknown */
		#error
	#endif

	#define MOSS_INLINE inline __attribute__((__always_inline__))
	#define MOSS_CEXPR  MOSS_INLINE
	#define MOSS_ND [[nodiscard]]
	#define MOSS_ND_INLINE [[nodiscard]] inline __attribute__((__always_inline__))
	#define MOSS_ND_CEXPR  MOSS_ND_INLINE
	#define MOSS_NOEXCEPT

	#if __MOSS_C__<2023 || defined __INTELLISENSE__
		#define alignof _Alignof
	#endif
	#if ( __MOSS_C__<2023 && !defined __clang__ ) || defined __INTELLISENSE__
		typedef _Bool bool;
		#define false ((_Bool)0)
		#define true  ((_Bool)1)
	#endif

	#define asm __asm__ //Clang needs this sometimes???   `asm` is in the standard!
#else
	#error
#endif

#ifdef __MOSS_CPP__
extern "C"
{
#endif



#if   defined __clang__
	#define __MOSS_CLANG__
	#define __moss_unreachable() __builtin_unreachable()
#elif defined __GNUC__
	#define __MOSS_GCC__
	#define __moss_unreachable() __builtin_unreachable()
#elif defined _MSC_VER
	#define __MOSS_MSVC__
	#define __moss_unreachable() __assume( false )
#else
	#error
#endif



#ifndef __INTELLISENSE__
	#define MOSS_VARFN_ATTRIB( PROTO, ARGNUM_FMT, ARGNUM_ARGS )\
		__attribute__(( format(PROTO,ARGNUM_FMT,ARGNUM_ARGS) ))
	#define MOSS_NORET_ATTRIB\
		__attribute__((noreturn))
#else
	#define MOSS_VARFN_ATTRIB( PROTO, ARGNUM_FMT, ARGNUM_ARGS )
	#define MOSS_NORET_ATTRIB
#endif

#ifndef NDEBUG /* debug */

	#include <stdarg.h>

	MOSS_NORET_ATTRIB void __moss_assert_fail1   (
		char const* file,int line, char const* fnname, char const* cond
	);
	MOSS_NORET_ATTRIB void __moss_assert_fail2   (
		char const* file,int line, char const* fnname, char const* fmt_cstr,...
	);
	MOSS_NORET_ATTRIB void __moss_assert_fail2_va(
		char const* file,int line, char const* fnname, char const* fmt_cstr,va_list args
	);

	#ifndef __INTELLISENSE__
		#define __moss_assert_false( FMT_CSTR,... )\
			__moss_assert_fail2(\
				__FILE__,__LINE__, __PRETTY_FUNCTION__, FMT_CSTR __VA_OPT__(,) __VA_ARGS__\
			)
		#define __moss_assert( PASS_COND, FMT_CSTR,... )\
			do\
			{\
				if (!(PASS_COND)) __moss_assert_false( FMT_CSTR __VA_OPT__(,) __VA_ARGS__ );\
			}\
			while (0)
	#else
		#define __moss_assert_false( FMT_CSTR,... )
		#define __moss_assert( PASS_COND, FMT_CSTR,... )
	#endif

	#define MOSS_DEBUG_ONLY( CODE ) CODE

	#define __moss_implerr __moss_assert_false("Implementation error!")
	#define __moss_notimpl __moss_assert_false("Not implemented!")

	#define MOSS_DEBUG

#else /* release */

	#define __moss_assert_false( FMT_CSTR,... ) __moss_unreachable()
	#define __moss_assert( PASS_COND, FMT_CSTR,... ) do {} while (0)

	#define MOSS_DEBUG_ONLY( CODE )

	#define __moss_implerr __moss_unreachable()
	#define __moss_notimpl __moss_unreachable()

	#define MOSS_RELEASE

#endif

#define __moss_assert_impl( PASS_COND ) __moss_assert( PASS_COND, "Implementation error!" )
#define __moss_assert_notimpl( PASS_COND ) __moss_assert( PASS_COND, "Not implemented!" )

#define __moss_switch_covered_enum //-Wswitch and -Wcovered-switch-default
#ifdef __MOSS_CPP__
	#define __moss_switch_default_unnecessary [[unlikely]] default: __moss_implerr
#else
	#define __moss_switch_default_unnecessary              default: __moss_implerr
#endif



// https://wiki.osdev.org/Stack_Smashing_Protector

#ifdef __MOSS_32__
extern unsigned int       __stack_chk_guard;
#else
extern unsigned long long __stack_chk_guard;
#endif



#ifdef __MOSS_CPP__
}
#endif
