#define MOSS_USE_WRONG_SIZE_T

#include "__env.h"




#if   MOSS_ENV == MOSS_ENV_USERLAND

	#error

#elif MOSS_ENV == MOSS_ENV_FREESTANDING

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
	#endif
	int _fltused;
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif

#elif MOSS_ENV == MOSS_ENV_EMBEDWIN

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
	#endif
	int _fltused;
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif



	#include "wchar.h"

	#define _WCHAR_T_DEFINED
	//#define __int32 int
	//#define __int64 long long
	//#define __unaligned __attribute__((aligned(1)))
	////#define __pragma(JUNK)
	//#define __declspec(JUNK)
	//#define __forceinline MOSS_INLINE
	//Also `MOSS_WRONG_SIZE_T` above
	#if   defined __MOSS_X86__
		#define _X86_
		//#define _M_IX86
	#elif defined __MOSS_AMD64__
		#define _AMD64_
		//#define _M_AMD64
		//#define _WIN64
	#else
		#error
	#endif

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		//#pragma clang diagnostic ignored "-Wcast-align"
		//#pragma clang diagnostic ignored "-Wcast-function-type-strict"
		#pragma clang diagnostic ignored "-Wcast-qual"
		#pragma clang diagnostic ignored "-Wgnu-offsetof-extensions"
		#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
		#pragma clang diagnostic ignored "-Wignored-attributes"
		#pragma clang diagnostic ignored "-Wignored-pragma-intrinsic"
		#pragma clang diagnostic ignored "-Wlanguage-extension-token"
		#pragma clang diagnostic ignored "-Wnonportable-include-path"
		#pragma clang diagnostic ignored "-Wpragma-pack"
		#pragma clang diagnostic ignored "-Wredundant-parens"
		#pragma clang diagnostic ignored "-Wstrict-prototypes"
		#pragma clang diagnostic ignored "-Wundef"
		//#pragma clang diagnostic ignored "-Wunknown-pragmas"
		//#pragma clang diagnostic ignored "-Wvisibility"
	#endif

	#include <libloaderapi.h>

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif



	static HMODULE _ucrtbased;

	typedef int(*__MOSS_Win_putchar)(int);
	typedef int(*__MOSS_Win_puts   )(char const*);
	typedef int(*__MOSS_Win_getch  )(void);
	typedef int(*__MOSS_Win_getchar)(void);

	typedef float (*__MOSS_Win_mathf)(float );
	typedef double(*__MOSS_Win_mathd)(double);

	static __MOSS_Win_putchar __moss_win_putchar;
	static __MOSS_Win_puts    __moss_win_puts   ;
	static __MOSS_Win_getchar __moss_win_getch  ;
	static __MOSS_Win_getchar __moss_win_getchar;

	static __MOSS_Win_mathf __moss_win_expf;
	static __MOSS_Win_mathd __moss_win_expd;
	static __MOSS_Win_mathf __moss_win_logf;

	static void __moss_ensure_env(void)
	{
		static int inited = 0;
		if ( !inited )
		{
			_ucrtbased = LoadLibraryA( "ucrtbased.dll" );

			#ifdef __MOSS_CLANG__
				#pragma clang diagnostic push
				#pragma clang diagnostic ignored "-Wcast-function-type"
			#endif

			__moss_win_putchar = (__MOSS_Win_putchar)GetProcAddress( _ucrtbased, "putchar" );
			__moss_win_puts    = (__MOSS_Win_puts   )GetProcAddress( _ucrtbased, "puts"    );
			__moss_win_getch   = (__MOSS_Win_getch  )GetProcAddress( _ucrtbased, "_getch"  );
			__moss_win_getchar = (__MOSS_Win_getchar)GetProcAddress( _ucrtbased, "getchar" );

			__moss_win_expf = (__MOSS_Win_mathf)GetProcAddress( _ucrtbased, "expf" );
			__moss_win_expd = (__MOSS_Win_mathd)GetProcAddress( _ucrtbased, "exp"  );
			__moss_win_logf = (__MOSS_Win_mathf)GetProcAddress( _ucrtbased, "logf" );

			#ifdef __MOSS_CLANG__
				#pragma clang diagnostic pop
			#endif

			inited = 1;
		}
	}

	int __moss_putc_stdout( int ch )
	{
		__moss_ensure_env();
		return __moss_win_putchar( ch );
	}
	int __moss_putc_stderr( int ch )
	{
		__moss_ensure_env();

		/* TODO: not this */
		return __moss_win_putchar( ch );

		//return ch;
	}

	MOSS_ND int __moss_getc_stdin(void)
	{
		__moss_ensure_env();
		return __moss_win_getchar();
	}
	MOSS_ND int __moss_getch(void)
	{
		__moss_ensure_env();
		int ch = __moss_win_getch();
		if ( ch == '\r' ) ch='\n'; // why, microsoft
		return ch;
	}

	MOSS_ND float  __moss_gt_expf( float  val )
	{
		__moss_ensure_env();
		return __moss_win_expf( val );
	}
	MOSS_ND double __moss_gt_expd( double val )
	{
		__moss_ensure_env();
		return __moss_win_expd( val );
	}
	MOSS_ND float  __moss_gt_logf( float  val )
	{
		__moss_ensure_env();
		return __moss_win_logf( val );
	}



	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
	#endif
	/*
	These strings can be used in Natvis expressions

	See also:
		https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects
		https://learn.microsoft.com/en-us/visualstudio/debugger/format-specifiers-in-cpp
		https://www.asciitable.com/

		s = ""
		for i in range(128):
			s += "\""+chr(i)+"\","
	*/
	char const* natvis_strs[ 128 + 1 + 10+10 ] =
	{
		//Inds [0,127] ASCII
		"\\x00","\\x01","\\x02","\\x03","\\x04","\\x05","\\x06","\\x07",
		"\\x08","\\t"  ,"\\n"  ,"\\x0b","\\x0c","\\r"  ,"\\x0e","\\x0f",
		"\\x10","\\x11","\\x12","\\x13","\\x14","\\x15","\\x16","\\x17",
		"\\x18","\\x19","\\x1a","\\x1b","\\x1c","\\x1d","\\x1e","\\x1f",
		" ","!","\"","#",   "$","%","&","'",   "(",")","*","+",   "," ,"-",".","/"    ,
		"0","1","2" ,"3",   "4","5","6","7",   "8","9",":",";",   "<" ,"=",">","?"    ,
		"@","A","B" ,"C",   "D","E","F","G",   "H","I","J","K",   "L" ,"M","N","O"    ,
		"P","Q","R" ,"S",   "T","U","V","W",   "X","Y","Z","[",   "\\","]","^","_"    ,
		"`","a","b" ,"c",   "d","e","f","g",   "h","i","j","k",   "l" ,"m","n","o"    ,
		"p","q","r" ,"s",   "t","u","v","w",   "x","y","z","{",   "|" ,"}","~","\\x7f",
		//Ind 128 empty string
		"",
		//Inds [129,138] subscripts
		"₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉",
		//Inds [139,139] superscripts
		"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"
	};
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif

#endif
