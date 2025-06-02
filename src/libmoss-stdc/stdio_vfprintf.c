#include "stdint.h" /* at top so e.g. `MOSS_INT_MAX` can't be disabled */

#include "__fprint.h"

#include "stddef.h" /* `ptrdiff_t` */
#include "stdio.h"

#include "alloca.h"
#include "assert.h"
#include "ctype.h"
#include "string.h"
#include "wchar.h"



/* TODO: floating-point, `int` overflows in these functions, "l" for "%c" and "%s" */



/*
https://en.cppreference.com/w/cpp/io/c/fprintf

Format specifiers are of the form:
	<fmt>    = "%" <flags>* <width>? <prec>? <length>? <spec>;

	<flags>  = "-"         (left-justify in field instead of default right-justify)
	         | "+"         (sign always prepended, not just when negative)
	         | " "         (prepend space instead of sign when pos; ignored if "+" flag too)
	         | "#"         ('alternate' form)
	         | "0"         (pad with leading zeros; ignore if also "-" flag or explicit <prec> on
	         ;             integers (they zero-pad to the precision instead))

	<width>  = <int>       (minimum field width to pad to)
	         | '*'         (same, but take width from an additional `int` arg preceding value)
	         ;

	<prec>   = "."         (set 'precision' to 0)
	         | "." <int>   (set 'precision' to value)
	         | ".*"        (set 'precision' to value from additional `int` arg preceding value;
	         ;             ignore if negative)

	<length> = "hh" | "h" | "l" | "j" | "z" | "t" | "L" ;   (modifies size of arg)

	<spec>   = "%"         (writes '%'; the full <fmt> must be "%%")
	         | "c"         (emits character; <length> "l" interprets input as `wchar_t`)
	         | "s"         (writes string; <length> "l" interprets input as `wchar_t*`; <prec> is
	        ...            maximum characters to write)
	         | "d" | "i"   (decimal int; <prec> is the min digits, default 1, padding 0s)
	         | "o"         (octal int; <prec> as for "d"; "#" ensures <prec> at least 1)
	         | "x"         (hex int, lowercase; <prec> as for "d"; "#" prepends "0x" iff nonzero)
	         | "X"         (as "x" but with uppercase)
	         | "u"         (unsigned int; <prec> as for "d")
	         | "f" | "F"   (decimal float; <prec> is number of digits after ".", default 6; "#"
	        ...            includes the "." even when <prec> is zero; infinity and nan are platform-
	        ...            specific "inf" and "nan")
	         | "e" | "E"   (sci float; as "f", but in scientific notation with at least two digits
	        ...            in exponent; "e" uses lowercase and "E" uses uppercase)
	         | "a" | "A"   (hex sci float; as "e"/"E", but no requirement on at least two digits)
	         | "g" | "G"   (selects between "f"/"e" and "F"/"E" depending on magnitude, then strips
	        ...            trailing zeros unless "#")
	         | "n"         (writes number of characters emitted so far into pointer argument; may
	        ...            not contain <flags>, <width>, or <prec>)
	         | "p"         (implementation-defined "pointer")
	         ;
*/



union __MOSS_PRINTF_FLAGS
{
	struct
	{
		uint8_t left_justify : 1;
		uint8_t sign_plus    : 1;
		uint8_t sign_space   : 1;
		uint8_t alternate    : 1;
		uint8_t pad_zeros    : 1;
		uint8_t              : 3;
	};
	uint8_t packed;
};



/*
Given a (pointer to a) string, which must start with a digit, extracts and returns a (nonnegative)
`int`, advancing the string.  If the result would overflow, do not advance string, and return -1.
*/
static int32_t __moss_parse_nonneg_int( char const** str_ptr )
{
	char const* str = *str_ptr;
	int32_t val=0, add;
	LOOP:
		add = *(str++) - '0';
		if ( val > INT32_MAX-add ) return -1;
		val += add;

		if (isdigit( *str ))
		{
			if ( val > INT32_MAX/10 ) return -1;
			val *= 10;

			goto LOOP;
		}

	*str_ptr = str;
	return val;
}

/* Given a formatted output in `str_prefix` + `str_value`, print it positioned in the field */
static int __moss_vfprintf_spec_helper1(
	FILE* stream,
	char const* str_prefix,
	size_t num_zeros_before, void const* str_value,int sz,size_t len_value, size_t num_zeros_after,
	union __MOSS_PRINTF_FLAGS flags,
	int pad_to_width
) {
	int ret;
	int count = 0;

	size_t len_prefix = strlen( str_prefix );
	/* note `len_value` can be less than `strlen(str_value)`, because it is clipped by <prec> */
	size_t len = len_prefix + num_zeros_before+len_value+num_zeros_after;

	int need_pad = pad_to_width!=-1 && len<(size_t)pad_to_width;
	int padding;
	char pad_ch;
	if ( need_pad )
	{
		assert( len < (size_t)pad_to_width );
		padding = (int)( (size_t)pad_to_width - len );
		pad_ch = flags.pad_zeros ? '0' : ' ';

		if ( !flags.left_justify )
		{
			ret = __moss_fputc_n( pad_ch, stream, padding );
			if ( ret == EOF ) return EOF;
			count += ret;
		}
	}

	ret = fputs( str_prefix, stream );
	if ( ret == EOF ) return EOF;
	count += ret;

	ret = __moss_fputc_n( '0', stream, (int)num_zeros_before );
	if ( ret == EOF ) return EOF;
	count += ret;

	/* Note not using `fputs(⋯)` because might be clipped */
	if ( len_value > 0 )
	{
		if ( sz == 1 )
		{
			char const* ptr = (char const*)str_value;
			for ( size_t i=0; i<len_value; ++i ) if ( fputc(ptr[i],stream) == EOF ) return EOF;
		}
		else
		{
			wchar_t const* ptr = (wchar_t const*)str_value;
			for ( size_t i=0; i<len_value; ++i ) if ( fputc(ptr[i],stream) == EOF ) return EOF;
		}
		count += (int)len_value;
	}

	ret = __moss_fputc_n( '0', stream, (int)num_zeros_after );
	if ( ret == EOF ) return EOF;
	count += ret;

	if ( need_pad && flags.left_justify )
	{
		#ifdef __MOSS_CLANG__
			#pragma clang diagnostic push
			#pragma clang diagnostic ignored "-Wconditional-uninitialized"
		#endif
		ret = __moss_fputc_n( pad_ch, stream, padding );
		#ifdef __MOSS_CLANG__
			#pragma clang diagnostic pop
		#endif
		if ( ret == EOF ) return EOF;
		count += ret;
	}

	return (int)count;
}

MOSS_ND_INLINE static int __moss_vfprintf_spec_helper2(
	char const**__restrict fmt_cstr_ptr, va_list*__restrict args_ptr,
	union __MOSS_PRINTF_FLAGS*__restrict flags, int32_t*__restrict width, int*__restrict prec, char* length
) {
	char ch;
	char const* fmt_cstr = *fmt_cstr_ptr;

	/* <flags> */
	#if 1
	flags->packed = 0;
	FLAGS:
	{
		ch = *fmt_cstr;
		switch ( ch )
		{
			case '-': flags->left_justify=1; ++fmt_cstr; goto FLAGS;
			case '+': flags->sign_plus   =1; ++fmt_cstr; goto FLAGS;
			case ' ': flags->sign_space  =1; ++fmt_cstr; goto FLAGS;
			case '#': flags->alternate   =1; ++fmt_cstr; goto FLAGS;
			case '0': flags->pad_zeros   =1; ++fmt_cstr; goto FLAGS;
			default: break;
		}
	}
	#endif

	/* <width> */
	#if 1
	*width = -1;
	ch = *fmt_cstr;
	if ( ch == '*' )
	{
		ch = *(++fmt_cstr);

		*width = (int32_t)va_arg( *args_ptr, int );
		if ( *width < 0 )
		{
			flags->left_justify = 1;
			if ( *width == INT32_MIN ) goto INVALID;
			*width = -*width;
		}
	}
	else if ( isdigit(ch) )
	{
		*width = __moss_parse_nonneg_int( &fmt_cstr );
		if ( *width < 0 ) goto INVALID;
		ch = *fmt_cstr;
	}
	assert( *width >= -1 );
	#endif

	/* <prec> */
	#if 1
	*prec = -1;
	if ( ch == '.' )
	{
		ch = *(++fmt_cstr);

		if ( isdigit(ch) )
		{
			*prec = __moss_parse_nonneg_int( &fmt_cstr );
			if ( *prec < 0 ) goto INVALID;
			ch = *fmt_cstr;
		}
		else if ( ch == '*' )
		{
			ch = *(++fmt_cstr);

			int prec_tmp = va_arg( *args_ptr, int );
			if ( prec_tmp >= 0 ) *prec=prec_tmp;
			/* ignored otherwise */
		}
		else
		{
			*prec = 0;
		}
	}
	assert( *prec >= -1 );
	#endif

	/* <length> */
	#if 1
	*length = ' ';
	switch ( ch ) {
		case 'h':
			ch = *(++fmt_cstr);
			if ( ch == 'h' )
			{
				ch = *(++fmt_cstr);
				*length = 'B'; /* "hh" */
			}
			else *length='S'; /* "h" */
			break;
		case 'l':
			ch = *(++fmt_cstr);
			if ( ch == 'l' )
			{
				ch = *(++fmt_cstr);
				*length = 'Q'; /* "ll" */
			}
			else *length='l'; /* "l" */
			break;
		case 'j': case 'z': case 't': case 'L':
			*length = ch; /* "j", "z", "t", "L" */
			ch = *(++fmt_cstr);
			break;
		default:
			break;
	}
	#endif

	*fmt_cstr_ptr = fmt_cstr;

	return 1;

	INVALID:
	return 0;
}
/*
Given a (pointer to) the format string right after a '%' has been encountered, executes the format
specifier, updating the string pointer, returning the number of characters written.  If the
specifier is invalid, prints nothing, extracts no arguments, and returns 0.  If write fails, it may
have partially printed, and `EOF` is returned.
*/
static int __moss_vfprintf_spec(
	FILE* stream, char const** fmt_cstr_ptr,va_list* args_ptr, int count_in_caller
) {
	int count = 0;

	/* Parse any <flags>, <width>, <prec>, and/or <length> */
	union __MOSS_PRINTF_FLAGS flags;
	int width, prec;
	char length;
	int ret = __moss_vfprintf_spec_helper2( fmt_cstr_ptr, args_ptr,
		&flags, &width, &prec, &length
	);
	if ( ret == 0 ) goto INVALID;

	char const* fmt_cstr = *fmt_cstr_ptr;
	char ch = *fmt_cstr;

	/* Parse <spec> and output */
	if      ( ch == '\0' ) goto INVALID;
	else if ( ch == '%'  )
	{
		if ( *fmt_cstr_ptr-fmt_cstr > 1 ) goto INVALID; /* Must be "%%" */
		++fmt_cstr;

		if ( fputc('%',stream) == EOF ) return EOF;
		++count;
	}
	else if ( ch == 's'  )
	{
		if (       /* 0b000'11110 */
			(flags.packed&0x1E) || (length!=' '&&length!='l')
		) goto INVALID;
		++fmt_cstr;

		void const* src;
		size_t len;
		if ( length != 'l' )
		{
			src = va_arg( *args_ptr, char const* );
			len = strlen( (char const*)src );
		}
		else
		{
			src = va_arg( *args_ptr, wchar_t const* );
			len = wcslen( (wchar_t const*)src );
		}
		if ( src == NULL ) src="(null)";

		if ( prec >= 0 && (size_t)prec<len ) len=(size_t)prec;

		ret = __moss_vfprintf_spec_helper1(
			stream, "", 0,(char const*)src,length=='l'?2:1,len,0, flags, width
		);
		if ( ret == EOF ) return EOF;
		count += ret;
	}
	else if ( ch == 'c'  )
	{
		if (       /* 0b000'11110 */
			(flags.packed&0x1E) || prec!=-1 || (length!=' '&&length!='l')
		) goto INVALID;
		++fmt_cstr;

		/* TODO: <length> "l" flag not implemented */
		union { char buf_c[2]; wchar_t buf_w[2]; } u;
		memset( &u, 0x00, sizeof(u) );
		if ( length != 'l' ) u.buf_c[0]=(char   )va_arg( *args_ptr, int );
		else                 u.buf_w[0]=(wchar_t)va_arg( *args_ptr, int );

		ret = __moss_vfprintf_spec_helper1(
			stream, "",0,u.buf_c,length=='l'?2:1,1,0, flags, width
		);
		if ( ret == EOF ) return EOF;
		count += ret;
	}
	else if ( ch == 'n'  )
	{
		if (
			flags.packed!=0 || width!=-1 || prec!=-1
		) goto INVALID;
		++fmt_cstr;

		switch ( length )
		{
			case 'B': *va_arg( *args_ptr, signed char* )=(signed char)count_in_caller; break;
			case 'S': *va_arg( *args_ptr, short      * )=(short      )count_in_caller; break;
			case ' ': *va_arg( *args_ptr, int        * )=(int        )count_in_caller; break;
			case 'l': *va_arg( *args_ptr, long       * )=(long       )count_in_caller; break;
			case 'Q': *va_arg( *args_ptr, long long  * )=(long long  )count_in_caller; break;
			case 'j': *va_arg( *args_ptr, intmax_t   * )=(intmax_t   )count_in_caller; break;
			case 'z': *va_arg( *args_ptr, size_t     * )=(size_t     )count_in_caller; break;
			case 't': *va_arg( *args_ptr, ptrdiff_t  * )=(ptrdiff_t  )count_in_caller; break;
			case 'L': default: goto INVALID;
		}
	}
	else if ( ch=='v' || ch=='V' )
	{
		if ( flags.packed != 0 ) goto INVALID;
		++fmt_cstr;

		char const* fmt_cstr_2 = va_arg( *args_ptr, char const* );
		__moss_assert( fmt_cstr_2!=NULL, "Format string cannot be null!" );

		if ( ch == 'v' ) // Variadic arguments
		{
			va_list args2;
			va_copy( args2, *args_ptr );
			#ifdef __MOSS_CLANG__
				#pragma clang diagnostic push
				#pragma clang diagnostic ignored "-Wformat-nonliteral"
			#endif
			ret = vfprintf( stream, fmt_cstr_2, args2 );
			#ifdef __MOSS_CLANG__
				#pragma clang diagnostic pop
			#endif
			va_end( args2 );
		}
		else // `va_list*` argument
		{
			va_list* args2 = va_arg( *args_ptr, va_list* );
			#ifdef __MOSS_CLANG__
				#pragma clang diagnostic push
				#pragma clang diagnostic ignored "-Wformat-nonliteral"
			#endif
			ret = vfprintf( stream, fmt_cstr_2, *args2 );
			#ifdef __MOSS_CLANG__
				#pragma clang diagnostic pop
			#endif
		}

		if ( ret == EOF ) return EOF;
		count += ret;
	}
	else
	{
		/* longest is "-0x" */
		char buf_prefix[ 4 ] = { '\0', '\0', '\0', '\0' };

		/*
		Largest output is negative max `double` in "%f":
			"179769313486231570814527423731704356798070567525844996598917476803157260780028538760589
			5586327668781715404589535143824642343213268894641827684675467035375169860499105765512820
			7624549009038932894407586850845513394230458323690322294816580855933212334827479782620414
			4723168738177180919299881250404026184124858368" (309)
		—plus the decimal point and plus the precision, e.g. ".000000"
		*/
		size_t bufval_sz = 309 + 1+(size_t)(prec!=-1?prec:6) + 1;
		char* buf_value = alloca( bufval_sz );
		memset( buf_value, 0x00, bufval_sz );

		int is_intlike;
		if      ( ch == 'p' )
		{
			if (       /* 0b000'00110 */
				(flags.packed&0x06) || prec!=-1 || length!=' '
			) goto INVALID;
			++fmt_cstr;

			is_intlike = 1;

			/* can't delegate to "x" / "X" because when 0 they don't print leading "0x" */

			if ( !flags.alternate )
			{
				buf_prefix[0] = '0';
				buf_prefix[1] = 'x';
			}

			char* dst = buf_value;

			uintptr_t val = va_arg( *args_ptr, uintptr_t );

			for ( int nyb=sizeof(uintptr_t)*2-1; nyb>=0; --nyb )
			{
				uintptr_t tmp = ( val >> 4*nyb ) & 0xF;
				*(dst++) = __moss_xchrs[ tmp ];
				// my extension: if '#' flag, add tick marks every four nybbles.
				if ( flags.alternate && nyb!=0 && nyb%4==0 ) *(dst++)='\'';
			}

			prec = 8*sizeof(uintptr_t) / 4;
			if ( flags.alternate ) prec+=sizeof(uintptr_t)/2-1 - 2;
		}
		else if (__moss_str_contains( "dioxXub", ch )) /* integer-like */
		{
			is_intlike = 1;

			if ( ch=='d' || ch=='i' )
			{
				if (     /* 0b000'01000 */
					flags.packed & 0x08
				) goto INVALID;
				++fmt_cstr;

				intmax_t val;
				switch ( length )
				{
					/* note some of these will be promoted */
					case 'B': val=va_arg( *args_ptr, /*signed char*/ int ); break;
					case 'S': val=va_arg( *args_ptr, /*short      */ int ); break;
					case ' ': val=va_arg( *args_ptr, int                 ); break;
					case 'l': val=va_arg( *args_ptr, long                ); break;
					case 'Q': val=va_arg( *args_ptr, long long           ); break;
					case 'j': val=va_arg( *args_ptr, intmax_t            ); break;
					case 't': val=va_arg( *args_ptr, ptrdiff_t           ); break;
					case 'z': ch='u'; goto SIZE_T;
					default: /*case 'L':*/ goto INVALID;
				}

				if ( val < 0 ) buf_prefix[0]='-';
				else
				{
					if      ( flags.sign_plus  ) buf_prefix[0]='+';
					else if ( flags.sign_space ) buf_prefix[0]=' ';
				}

				if ( val != 0 )
				{
					/* write integer backward, then reverse */
					char* dst = buf_value;

					if ( val == INTMAX_MIN )
					{
						/*
						We need a bit of help because we can't just negate it.  Output the lowest
						digit first.  Value is of the form -2ʰ⁻¹, where h∈{8,16,32,64}.  So, the
						last digit is an '8':
							-128 -32768, -2147483648, -9223372036854775808
						*/
						*(dst++) = '8';
						val = ( val + 8 ) / 10;
					}
					if ( val < 0 ) val=-val;

					while ( val > 0 )
					{
						*(dst++) = '0' + val%10;
						val /= 10;
					}

					__moss_reverse( buf_value, dst );
				}
				else if ( prec != 0 ) buf_value[0]='0';
			}
			else /* "o", "x"/"X", "u", "b" */
			{
				if (     /* 0b000'00110 */
					(flags.packed&0x06) || (ch=='u'&&flags.alternate)
				) goto INVALID;
				++fmt_cstr;

				uintmax_t val;
				switch ( length )
				{
					/* note some of these will be promoted */
					case 'B':         val=(uintmax_t)va_arg( *args_ptr, /*unsigned char */ int ); break;
					case 'S':         val=(uintmax_t)va_arg( *args_ptr, /*unsigned short*/ int ); break;
					case ' ':         val=(uintmax_t)va_arg( *args_ptr, unsigned int           ); break;
					case 'l':         val=(uintmax_t)va_arg( *args_ptr, unsigned long          ); break;
					case 'Q':         val=(uintmax_t)va_arg( *args_ptr, unsigned long long     ); break;
					case 'j':         val=(uintmax_t)va_arg( *args_ptr, uintmax_t              ); break;
					case 't':         val=(uintmax_t)va_arg( *args_ptr, uintptr_t              ); break;
					case 'z': SIZE_T: val=(uintmax_t)va_arg( *args_ptr, size_t                 ); break;
					default: /*case 'L':*/ goto INVALID;
				}

				if ( val != 0 )
				{
					/* write integer backward, then reverse */
					char* dst = buf_value;

					if      ( ch == 'u' )
					{
						while ( val > 0 )
						{
							*(dst++) = '0' + val%10;
							val /= 10;
						}
					}
					else if ( ch == 'o' )
					{
						while ( val > 0 )
						{
							*(dst++) = __moss_xchrs[ val & 0x07 ];
							val >>= 3;
						}
						if ( flags.alternate ) *(dst++)='0'; /* leading zero */
					}
					else if ( ch == 'b' ) /* extension (seen in other implementations too) */
					{
						if ( flags.alternate )
						{
							buf_prefix[0] = '0';
							buf_prefix[1] = 'b';
						}

						while ( val > 0 )
						{
							*(dst++) = '0' + (val&0x01);
							val >>= 1;
						}
					}
					else /* "x", "X" */
					{
						assert( ch=='x' || ch=='X' );

						if ( flags.alternate )
						{
							buf_prefix[0] = '0';
							buf_prefix[1] = 'x';
						}

						char const* syms = ch=='x' ? __moss_xchrs : __moss_Xchrs;
						while ( val > 0 )
						{
							*(dst++) = syms[ val & 0x0F ];
							val >>= 4;
						}
					}

					__moss_reverse( buf_value, dst );
				}
				else if ( prec != 0 ) buf_value[0]='0';
			}
		}
		else if (__moss_str_contains( "fFeEaAgG", ch )) /* float-like */
		{
			++fmt_cstr;

			is_intlike = 0;

			long double val;
			if ( length != 'L' ) val=(long double)va_arg( *args_ptr,      double );
			else                 val=             va_arg( *args_ptr, long double );

			int tidy = !flags.alternate && ( ch=='g' || ch=='G' );

			FILE file_str = __moss_file_str( buf_value, bufval_sz );
			__moss_fprint_fp( &file_str, (double)val, ch,prec, flags.alternate );

			if ( tidy ) /* remove trailing zeros and decimal point */
			{
				/*size_t last = strlen( buf_value ) - 1;
				__moss_str_contains( buf_value, '.' );
				if ( buf_value[last] == '0' )
				bufval_sz*/
				assert(0); /* TODO */
			}
		}
		else goto INVALID;

		size_t len_value = strlen(buf_value);
		size_t num_zeros_before=0, num_zeros_after=0;
		if ( prec > 0 )
		{
			if ( is_intlike )
			{
				if ( (size_t)prec > len_value ) num_zeros_before=(size_t)prec-len_value;
			}
			else num_zeros_after=0; // already added by `__moss_ftoa(⋯)`
		}

		ret = __moss_vfprintf_spec_helper1(
			stream,
			buf_prefix, num_zeros_before,buf_value,1,len_value,num_zeros_after,
			flags, width
		);
		if ( ret == EOF ) return EOF;
		count += ret;
	}

	/* success */
	*fmt_cstr_ptr = fmt_cstr;
	return count;

	INVALID:
	return 0;
}



#if 1
MOSS_VARFN_ATTRIB( printf, 2, 0 )
int vfprintf( FILE* stream, char const* fmt_cstr,va_list args )
{
	va_list args2;
	va_copy( args2, args );

	int count = 0;

	LOOP:
	{
		char ch = *fmt_cstr;

		if ( ch != '\0' )
		{
			++fmt_cstr;

			if ( ch != '%' )
			{
				int ret = fputc( ch, stream );
				if ( ret == EOF ) { count=EOF; goto DONE; }
				++count;
			}
			else
			{
				int ret = __moss_vfprintf_spec( stream, &fmt_cstr,&args2, count );
				if ( ret == EOF ) { count=EOF; goto DONE; }
				/* Note `ret` can be zero e.g. if <prec> zero clipped an int; it's not an error */
				count += ret;
			}

			goto LOOP;
		}
	}

	DONE:

	va_end( args2 );

	return count;
}
#endif
