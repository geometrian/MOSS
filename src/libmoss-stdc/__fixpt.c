#include "__fixpt.h"

#include <inttypes.h>

#include "math.h"
#include "string.h"



static void _fixpt_bcd1076_renormalize( _MOSS_FixPtBCD1076* bcd )
{
	uint16_t trailing_zeros = 0;
	for ( int k=0; k<bcd->decpt_ind; ++k )
	{
		if ( bcd->digits[k] != 0 ) break;
		++trailing_zeros;
	}
	if ( trailing_zeros == 0 ) return;

	__moss_assert_impl( trailing_zeros<bcd->digits_count && trailing_zeros<=bcd->decpt_ind );
	__moss_fixpt_bcd1076_truncate( bcd, trailing_zeros );
}

void __moss_fixpt_bcd1076_zero( _MOSS_FixPtBCD1076* bcd )
{
	bcd->is_neg = 0;

	bcd->decpt_ind = 0;

	bcd->digits_count = 1;
	bcd->digits[0] = 0;
	#ifdef MOSS_DEBUG
	for ( int k=1; k<1076; ++k ) bcd->digits[k]=255;
	#endif
}

void __moss_fixpt_bcd1076_truncate( _MOSS_FixPtBCD1076* bcd, uint16_t num_digits_to_remove )
{
	__moss_assert( num_digits_to_remove<bcd->digits_count, "Cannot remove all digits!" );

	memmove(
		bcd->digits, bcd->digits+num_digits_to_remove,
		bcd->digits_count - num_digits_to_remove
	);
	bcd->decpt_ind    -= num_digits_to_remove;
	bcd->digits_count -= num_digits_to_remove;

	#ifdef MOSS_DEBUG
	for ( int k=bcd->digits_count; k<bcd->digits_count+num_digits_to_remove; ++k )
	{
		bcd->digits[k] = 255;
	}
	#endif
}

void __moss_fixpt_bcd1076_mul_u60( _MOSS_FixPtBCD1076* bcd, uint64_t val60 )
{
	// Needed so multiplication below can't overflow
	__moss_assert( val60<(2ull<<60), "Invalid value %" PRIu64 "!", val60 );

	if ( val60 == 0 )
	{
		__moss_fixpt_bcd1076_zero( bcd );
		return;
	}

	uint64_t sum = 0;
	for ( int i=0; i<bcd->digits_count; ++i )
	{
		// 60 bits since (2⁶⁴-1) - 9∙(2⁶⁰-1) ≥ 0
		sum += val60 * bcd->digits[i];
		bcd->digits[i] = sum % 10;
		sum /= 10;
	}
	while ( sum > 0 )
	{
		__moss_assert( bcd->digits_count<1076, "Too many digits!" );
		bcd->digits[ bcd->digits_count++ ] = sum % 10;
		sum /= 10;
	}

	_fixpt_bcd1076_renormalize( bcd );
}
void __moss_fixpt_bcd1076_div_2( _MOSS_FixPtBCD1076* bcd )
{
	// Add new 0 digit if necessary
	if ( bcd->digits[0]%2 == 1 )
	{
		__moss_fixpt_bcd1076_append_frac_zero( bcd );
	}

	uint8_t* digit = bcd->digits + bcd->digits_count-1;
	uint8_t dividend = 0;
	LOOP:
		dividend += *digit;
		uint8_t quotient  = dividend / 2;
		uint8_t remainder = dividend % 2;
		*digit = quotient;
		if ( --digit >= bcd->digits )
		{
			dividend = 10 * remainder;
			goto LOOP;
		}
}

void __moss_fixpt_bcd1076_append_frac_zero( _MOSS_FixPtBCD1076* bcd )
{
	__moss_assert( bcd->digits_count<1076, "Too many digits!" );
	memmove(
		bcd->digits+1, bcd->digits,
		bcd->digits_count
	);
	bcd->digits[0] = 0;
	++bcd->decpt_ind;
	++bcd->digits_count;
}

void __moss_fixpt_bcd1076_round_to( _MOSS_FixPtBCD1076* bcd, uint16_t max_frac_digits )
{
	if ( max_frac_digits >= bcd->decpt_ind ) return;

	uint8_t rounding_digit = bcd->digits[ bcd->decpt_ind - max_frac_digits - 1 ];

	__moss_fixpt_bcd1076_truncate( bcd, bcd->decpt_ind-max_frac_digits );

	if ( rounding_digit < 5 ) return;

	uint8_t* digit = bcd->digits;
	LOOP:
		if ( ++*digit < 10 ) return;
		*digit = 0;
		++digit;
		if ( digit < bcd->digits+bcd->digits_count ) goto LOOP;

		__moss_assert( bcd->digits_count<1076, "Too many digits!" );
		bcd->digits[ bcd->digits_count++ ] = 1;
}

void __moss_fixpt_bcd1076_from_f64( _MOSS_FixPtBCD1076* bcd, double finite_val )
{
	__MOSS_Flt64 bits;
	bits.as_f64 = finite_val;

	__moss_assert( bits.biasexp!=0x7FF, "Invalid non-finite value %f!", finite_val );

	// Store the decimal representation of the unit in last place
	int power = (int)bits.biasexp - 1023 - 52;
	__moss_fixpt_bcd1076_zero( bcd );
	bcd->digits[0] = 1;
	if      ( power < 0 )
	{
		do
		{
			__moss_fixpt_bcd1076_div_2( bcd );
			++power;
		}
		while ( power < 0 );
	}
	else if ( power > 0 )
	{
		do
		{
			__moss_fixpt_bcd1076_mul_u60( bcd, 2 );
			--power;
		}
		while ( power > 0 );
	}

	// Now multiply by the coefficient
	uint64_t coeff = bits.mantissa;
	if ( bits.biasexp != 0 ) coeff|=1ull<<52;
	__moss_fixpt_bcd1076_mul_u60( bcd, coeff );

	bcd->is_neg = bits.sign;
}
