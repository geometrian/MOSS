#include "math.h"

#include "stdint.h"



#if 0 //Doesn't converge fast enough

/* See "Matters Computational" §29.1.3 */

MOSS_ND static double __moss_pow_negrecip3( double val ) MOSS_NOEXCEPT
{
	double x = val;
	unsigned iters;
	for ( iters=0; iters<20; ++iters )
	{
		double xnext = __moss_fmad(
			__moss_fmad( -val*x,x*x, 1.0 ),
			(1.0/3.0)*x,
			x
		);
		if ( xnext == x ) break;
		x = xnext;
	}
	return x;
}

MOSS_ND double __moss_cbrt( double val ) MOSS_NOEXCEPT
{
	return val * __moss_pow_negrecip3( val*val );
}

#endif



/*
Similarly to the bithack trick for fast inverse-square-root and fast square-root:

	x⁺¹⸍³ = exp₂(log₂(x⁺¹⸍³)) = exp₂(+⅓ log₂(x))
	x⁻¹⸍³ = exp₂(log₂(x⁻¹⸍³)) = exp₂(-⅓ log₂(x))

Where exp₂(⋯) and log₂(⋯) can be quickly approximated by (interpreting the bits of input x as an
integer xᵢₙₜ):

	log₂(x) ≈ xᵢₙₜ/2²³ -  127   (32-bit)
	log₂(x) ≈ xᵢₙₜ/2⁵² - 1023   (64-bit)

Thus for 64-bit:

	x⁺¹⸍³ ≈ ( (xᵢₙₜ/2⁵² - 1023)/3 + 1023 ) 2⁵²
	      = xᵢₙₜ/3 - 2⁵²1023/3 + 2⁵²1023
	      = xᵢₙₜ/3 + 2⁵²(1023 - 1023/3)

	x⁻¹⸍³ ≈ ( (xᵢₙₜ/2⁵² - 1023)/-3 + 1023 ) 2⁵²
	      = -xᵢₙₜ/3 + 2⁵²1023/3 + 2⁵²1023
	      = 2⁵²(1023 + 1023/3) - xᵢₙₜ/3
*/
MOSS_ND_INLINE static double __moss_pow_negrecip3( double val ) MOSS_NOEXCEPT
{
	__MOSS_Flt64 bits = __moss_f64_to_bits( val );
	uint64_t const TWEAK = 0;
	uint64_t const MAGIC = (1ull<<52)*( 1023 + 1023/3 ) + TWEAK;
	bits.as_u64 = MAGIC - bits.as_u64/3;
	double ret = __moss_bits_to_f64( bits );

	for ( int i=0; i<4; ++i )
	{
		ret = __moss_fmad(
			__moss_fmad( -val*ret,ret*ret, 1.0 ),
			(1.0/3.0)*ret,
			ret
		);
	}

	return ret;
}
MOSS_ND double __moss_cbrt( double val ) MOSS_NOEXCEPT
{
	// The N–R iteration for cube root involves a division, so instead of doing it directly, compute
	// x¹⸍³ = x (x²)⁻¹⸍³
	return val * __moss_pow_negrecip3( val * val );
}
