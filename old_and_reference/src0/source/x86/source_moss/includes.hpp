#pragma once


//#include <functional>
//#include <limits>
//Note cannot be e.g. <cstddef>; cross-compilers evidently do not support that.
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define restrict __restrict


#define MOSS_DEBUG

#ifdef MOSS_DEBUG
	#define MOSS_DEBUG_BOCHSBREAK __asm__ __volatile__("xchg  %bx, %bx")
	#define DEBUG_ONLY(CODE) CODE
	#define RELEASE_ONLY(CODE)
	#define DEB_REL_CODE(CODE_DEB,CODE_REL) CODE_DEB
#else
	#define MOSS_DEBUG_BOCHSBREAK
	#define DEBUG_ONLY(CODE)
	#define RELEASE_ONLY(CODE) CODE
	#define DEB_REL_CODE(CODE_DEB,CODE_REL) CODE_REL
#endif

#define COMMA ,
#define NOTHING

#if !defined(__cplusplus)
	#error "C++ should be used!"
#endif
#if defined(__linux__)
	#error "You are not using a cross-compiler!"
#endif

extern "C" void die(void);

namespace MOSS {
	//Assertion code adapted from libib.

	void _message(char const* filename,int line, char const* fmt_cstr,va_list args);

	void stack_trace(void);

	#ifdef MOSS_DEBUG
		inline void _assert_warn(bool pass_condition, char const* filename,int line, char const* fmt_cstr,...) { if (!pass_condition) { va_list args; va_start(args,fmt_cstr);
			_message(filename,line,fmt_cstr,args);
		va_end(args);        }}
		inline void _assert_term(bool pass_condition, char const* filename,int line, char const* fmt_cstr,...) { if (!pass_condition) { va_list args; va_start(args,fmt_cstr);
			_message(filename,line,fmt_cstr,args);
		va_end(args); die(); }}

		inline void _assertw_warn(bool pass_condition, char const* filename,int line, char const* fmt_cstr,...) { if (!pass_condition) { va_list args; va_start(args,fmt_cstr);
			_message(filename,line,fmt_cstr,args);
		va_end(args);        }}
		inline void _assertw_term(bool pass_condition, char const* filename,int line, char const* fmt_cstr,...) { if (!pass_condition) { va_list args; va_start(args,fmt_cstr);
			_message(filename,line,fmt_cstr,args);
		va_end(args); die(); }}
	#endif

	#define assertr_term(PASS_CONDITION, FMT_CSTR,...) MOSS::_assert_term(PASS_CONDITION, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
	#define assertr_warn(PASS_CONDITION, FMT_CSTR,...) MOSS::_assert_warn(PASS_CONDITION, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
	#define assertrw_term(PASS_CONDITION, FMT_CSTR,...) MOSS::_assertw_term(PASS_CONDITION, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
	#define assertrw_warn(PASS_CONDITION, FMT_CSTR,...) MOSS::_assertw_warn(PASS_CONDITION, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
	#ifdef MOSS_DEBUG
		#define assert_term(PASS_CONDITION, FMT_CSTR,...) MOSS::_assert_term(PASS_CONDITION, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
		#define assert_warn(PASS_CONDITION, FMT_CSTR,...) MOSS::_assert_warn(PASS_CONDITION, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
		#define assertw_term(PASS_CONDITION, FMT_CSTR,...) MOSS::_assertw_term(PASS_CONDITION, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
		#define assertw_warn(PASS_CONDITION, FMT_CSTR,...) MOSS::_assertw_warn(PASS_CONDITION, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
		#define asserts_term(PASS_EXPR,CHECK_EXPR, FMT_CSTR,...) MOSS::_assert_term(PASS_EXPR CHECK_EXPR, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
		#define asserts_warn(PASS_EXPR,CHECK_EXPR, FMT_CSTR,...) MOSS::_assert_warn(PASS_EXPR CHECK_EXPR, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
		#define assertsw_term(PASS_EXPR,CHECK_EXPR, FMT_CSTR,...) MOSS::_assertw_term(PASS_EXPR CHECK_EXPR, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
		#define assertsw_warn(PASS_EXPR,CHECK_EXPR, FMT_CSTR,...) MOSS::_assertw_warn(PASS_EXPR CHECK_EXPR, __FILE__,__LINE__, FMT_CSTR, ##__VA_ARGS__)
	#else
		#define assert_term(PASS_CONDITION, FMT_CSTR,...) {}
		#define assert_warn(PASS_CONDITION, FMT_CSTR,...) {}
		#define assertw_term(PASS_CONDITION, FMT_CSTR,...) {}
		#define assertw_warn(PASS_CONDITION, FMT_CSTR,...) {}
		#define asserts_term(PASS_EXPR,CHECK_EXPR, FMT_CSTR,...) PASS_EXPR
		#define asserts_warn(PASS_EXPR,CHECK_EXPR, FMT_CSTR,...) PASS_EXPR
		#define assertsw_term(PASS_EXPR,CHECK_EXPR, FMT_CSTR,...) PASS_EXPR
		#define assertsw_warn(PASS_EXPR,CHECK_EXPR, FMT_CSTR,...) PASS_EXPR
	#endif
}
