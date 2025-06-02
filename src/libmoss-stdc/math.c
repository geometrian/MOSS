#include "math.h"

#include "assert.h"



static_assert( sizeof(__MOSS_Flt32)==4 && sizeof(float )==4, "Invalid floating-point!" );
static_assert( sizeof(__MOSS_Flt64)==8 && sizeof(double)==8, "Invalid floating-point!" );



MOSS_ND float  frexpf( float  val, int* power ) MOSS_NOEXCEPT
{
	if ( val == 0.0f ) { *power=0; return val; }
	__MOSS_Flt32 bits = __moss_f32_to_bits( val );
	if ( bits.biasexp == 255/*0b11111111*/ ) { *power=0; return val; } /* inf or NaN */

	if ( bits.biasexp != 0 ) /* normal */
	{
		*power = bits.biasexp - 127 + 1;
	}
	else /* subnormal */
	{
		*power = -126 + 1;
		__moss_assert_impl( bits.mantissa != 0 );
		unsigned mant = bits.mantissa;
		while ( mant < (1u<<23) ) { mant<<=1; --*power; }
		bits.mantissa = mant; /* note cutting off new leading 1 */
	}
	bits.biasexp = 127 - 1;

	return __moss_bits_to_f32( bits );
}
MOSS_ND double frexp ( double val, int* power ) MOSS_NOEXCEPT
{
	if ( val == 0.0 ) { *power=0; return val; }
	__MOSS_Flt64 bits = __moss_f64_to_bits( val );
	if ( bits.biasexp == 2047/*0b11111111111*/ ) { *power=0; return val; } /* inf or NaN */

	if ( bits.biasexp != 0 ) /* normal */
	{
		*power = bits.biasexp - 1023 + 1;
	}
	else /* subnormal */
	{
		*power = -1022 + 1;
		__moss_assert_impl( bits.mantissa != 0 );
		unsigned long long mant = bits.mantissa;
		while ( mant < (1ull<<52) ) { mant<<=1; --*power; }
		bits.mantissa = mant; /* note cutting off new leading 1 */
	}
	bits.biasexp = 1023 - 1;

	return __moss_bits_to_f64( bits );
}
