#include "__fprint.h"

#include "__fixpt.h"
#include "assert.h"
#include "stdio.h"
#include "string.h"



/*
Checks for NaN and infinity, and returns `bits`, a positive number.  Returns number of characters
written (or EOF if write error).  Caller should continue printing the `val` if the result is 1 or
less.
*/
static int _handle_special( double val, FILE* stream, __MOSS_Flt64* bits )
{
	int n=0, ret;

	bits->as_f64 = val;

	if ( bits->sign )
	{
		ret = fputc( '-', stream );
		if ( ret == EOF ) return EOF;
		++n;

		bits->sign = 0;
	}

	if      ( isinf(val) )
	{
		ret = fputc( 'i', stream );
		if ( ret == EOF ) return EOF;
		ret = fputc( 'n', stream );
		if ( ret == EOF ) return EOF;
		ret = fputc( 'f', stream );
		if ( ret == EOF ) return EOF;
		n += 3;
	}
	else if ( isnan(val) )
	{
		ret = fputc( 'n', stream );
		if ( ret == EOF ) return EOF;
		ret = fputc( 'a', stream );
		if ( ret == EOF ) return EOF;
		ret = fputc( 'n', stream );
		if ( ret == EOF ) return EOF;
		n += 3;
	}

	return n;
}



int __moss_fprint_fp_dec( FILE* stream, double val, char specifier,int prec,bool always_pt )
{
	__moss_assert( specifier=='f' || specifier=='F',
		"Invalid printf-like specifier for hex floating-point number '%c'!", specifier
	);
	__moss_assert( prec>=-1, "Invalid `prec`=%d in floating-point print!", prec );

	__MOSS_Flt64 bits;
	int ret = _handle_special( val, stream, &bits );
	if ( ret==EOF || ret>1 ) return ret; // EOF, 3, or 4
	int n = ret;

	if ( prec == -1 ) prec=6;

	_MOSS_FixPtBCD1076 bcd;
	__moss_fixpt_bcd1076_from_f64( &bcd, val );
	__moss_fixpt_bcd1076_round_to( &bcd, (uint16_t)prec );

	// Output integer part
	int k;
	for ( k=bcd.digits_count-1; k>=bcd.decpt_ind; --k )
	{
		ret = fputc( '0'+bcd.digits[k], stream );
		if ( ret == EOF ) return ret;
		++n;
	}

	// Output decimal point
	if ( prec>0 || always_pt )
	{
		ret = fputc( '.', stream );
		if ( ret == EOF ) return ret;
		++n;
	}

	// Output fractional part

	for ( ; k>=0&&prec>0; --k,--prec )
	{
		ret = fputc( '0'+bcd.digits[k], stream );
		if ( ret == EOF ) return ret;
		++n;
	}
	for ( ; prec>0; --prec )
	{
		ret = fputc( '0', stream );
		if ( ret == EOF ) return ret;
		++n;
	}

	return n;
}

/*int __moss_sprint_fp_exp( FILE* stream, double val, char specifier,int prec,bool always_pt )
{
	
}*/

int __moss_fprint_fp_hex( FILE* stream, double val, char specifier,int prec,bool always_pt )
{
	__moss_assert( specifier=='a' || specifier=='A',
		"Invalid printf-like specifier for hex floating-point number '%c'!", specifier
	);
	__moss_assert( prec>=-1, "Invalid `prec`=%d in floating-point print!", prec );

	__MOSS_Flt64 bits;
	int ret = _handle_special( val, stream, &bits );
	if ( ret==EOF || ret>1 ) return ret; // EOF, 3, or 4
	int n = ret;

	char const* xchrs = specifier=='a' ? __moss_xchrs : __moss_Xchrs;

	// Construct the full coefficient by adding the implicit leading 1, if there
	uint64_t coeff = bits.mantissa;
	if ( bits.biasexp != 0 ) coeff|=1ull<<52;

	// Round to precision if specified
	if ( prec>=0 && prec<13 )
	{
		// Check the nybble we're going to cut off, and use it to round the rest up, if necessary.
		// Notice we do this on the fully constructed coefficient, so that carrying works.  In
		// principle, this could even result in a leading 2.

		int shift = 4 * ( 12 - prec );
		coeff >>= shift;

		uint8_t nyb = (uint8_t)( coeff & 0xF );
		if ( nyb >= 0x8 ) coeff+=0x10;
		// coeff &= ~0xFull; // don't bother

		coeff <<= shift;
	}
	// Remove trailing zeros, if in default mode
	if ( prec == -1 )
	{
		if ( coeff == 0 ) prec=0; // Strip all trailing zeros for value 0
		else
		{
			prec = 13;
			uint64_t mask = 0xF;
			while ( prec>0 && (coeff&mask)==0 )
			{
				--prec;
				mask <<= 4;
			}
		}
	}

	// Output e.g. "0x" or "0X"
	ret = fputc( '0', stream );
	if ( ret == EOF ) return EOF;
	ret = fputc( specifier+('x'-'a'), stream ); //'x' or 'X'
	if ( ret == EOF ) return EOF;
	// (increment `n` below)

	// Output leading "1" or "2"
	ret = fputc( xchrs[(coeff>>52)&0xF], stream );
	if ( ret == EOF ) return EOF;
	n += 3;

	// Output fraction
	if ( prec>0 || always_pt )
	{
		ret = fputc( '.', stream );
		if ( ret == EOF ) return EOF;
		++n;

		for ( int nyb=12; 12-nyb<prec; --nyb ) // Note this handle `prec` > 13 too
		{
			ret = fputc( xchrs[(coeff>>4*nyb)&0xF], stream );
			if ( ret == EOF ) return EOF;
			++n;
		}
	}

	// Output 'p' or 'P'
	ret = fputc( specifier+('p'-'a'), stream ); //'p' or 'P'
	if ( ret == EOF ) return EOF;
	++n;

	// Output '+' or '-'
	int power = bits.biasexp - 1023;
	if ( power==-1023 && bits.mantissa==0 ) power=0;
	char pm = '+';
	if ( power < 0 )
	{
		pm = '-';
		power = -power;
	}
	ret = fputc( pm, stream );
	if ( ret == EOF ) return EOF;
	++n;

	// Output exponent
	ret = __moss_fprint_dec_u( stream, (uint64_t)power, -1, false );
	if ( ret == EOF ) return EOF;
	n += ret;

	return n;
}

int __moss_fprint_fp( FILE* stream, double val, char specifier,int prec,bool always_pt )
{
	__moss_assert( __moss_str_contains("fFeEaAgG",specifier),
		"Invalid printf-like specifier for floating-point number '%c'!", specifier
	);
	__moss_assert( prec>=-1, "Invalid `prec`=%d in floating-point print!", prec );

	if      ( specifier=='f' || specifier=='F' )
	{
		return __moss_fprint_fp_dec( stream, val, specifier, prec, always_pt );
	}
	else if ( specifier=='e' || specifier=='E' )
	{
		//return __moss_fprint_fp_exp( stream, val, specifier, prec, always_pt );
		__moss_notimpl;
	}
	else if ( specifier=='a' || specifier=='A' )
	{
		return __moss_fprint_fp_hex( stream, val, specifier, prec, always_pt );
	}
	else __moss_notimpl;
}
