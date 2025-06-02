#include "math.h"

#include "stdint.h"



MOSS_ND double __moss_log( double val ) MOSS_NOEXCEPT
{
	if ( val<0.0 || __moss_isnan_f64(val) ) return __moss_nand();
	if ( __moss_isinf_f64(val) ) return val;

	int power;
	val = frexp( val, &power );

	/*
	Now, we need to compute:

		ln(`val`∙(2^`power`)) = ln(`val`) + `power` ln(2)

	Since `val` in range [½,1), can compute the first term using the series for ln(1+x), where
	x=`val`-1, which is convergent for -1<x≤1 (and, it is).  The latter term is just multiplication
	by a constant.

	In practice, the range on `val` means it doesn't converge impressively.  Taking the series for
	ln(1+x) at x=-¼ (the same as ln(x) at ¾) helps, but because it is no longer centered on 0, cases
	like ln(1) have noticeably bad accuracy.

	TODO: reduce by nth root, from Matters Computational §32.1.3, and use much smaller series.  Or
	AGM: https://www.reddit.com/r/mathematics/comments/cpknmk/this_formula_to_compute_the_natural_log_of_any/
	https://en.wikipedia.org/wiki/Logarithm#Feynman's_algorithm

	https://en.wikipedia.org/wiki/List_of_logarithmic_identities
	*/
	double result;
	{
		#if 1 // Taylor series ln(1+x) at 0
		#if 0
		double x1 = val - 1.0; // in range [-½,0)
		#else
		--power; val*=2.0;
		double x1 = val - 1.0; // in range [-½,½) . . . worse, but makes zero come out perfect
		#endif

		double x2 = x1 * x1;

		double x3 = x2 * x1;
		double x4 = x2 * x2;

		double x5 = x2 * x3;
		double x6 = x3 * x3;
		double x7 = x4 * x3;
		double x8 = x4 * x4;

		double x9  = x5 * x4;
		double x10 = x5 * x5;
		double x11 = x6 * x5;
		double x12 = x6 * x6;
		double x13 = x7 * x6;
		double x14 = x7 * x7;
		double x15 = x8 * x7;
		double x16 = x8 * x8;

		result =
			  x1
			- x2  * (1.0/ 2.0)
			+ x3  * (1.0/ 3.0)
			- x4  * (1.0/ 4.0)
			+ x5  * (1.0/ 5.0)
			- x6  * (1.0/ 6.0)
			+ x7  * (1.0/ 7.0)
			- x8  * (1.0/ 8.0)
			+ x9  * (1.0/ 9.0)
			- x10 * (1.0/10.0)
			+ x11 * (1.0/11.0)
			- x12 * (1.0/12.0)
			+ x13 * (1.0/13.0)
			- x14 * (1.0/14.0)
			+ x15 * (1.0/15.0)
			- x16 * (1.0/16.0)
		;
		#endif
		#if 0 // Taylor series ln(1+x) at -¼, same as ln(x) at ¾
		double y1  = val - 0.75; // x=val-1, y=x+¼

		double y2  = y1 * y1;

		double y3  = y2 * y1;
		double y4  = y2 * y2;

		double y5  = y2 * y3;
		double y6  = y3 * y3;
		double y7  = y4 * y3;
		double y8  = y4 * y4;

		double y9  = y5 * y4;
		double y10 = y5 * y5;
		double y11 = y6 * y5;
		double y12 = y6 * y6;
		double y13 = y7 * y6;
		double y14 = y7 * y7;
		double y15 = y8 * y7;
		double y16 = y8 * y8;

		result =
			-0.2876820724517809
			+1.3333333333333333 * y1
			-0.8888888888888888 * y2
			+0.7901234567901234 * y3
			-0.7901234567901234 * y4
			+0.842798353909465  * y5
			-0.9364426154549611 * y6
			+1.070220131948527  * y7
			-1.2485901539399482 * y8
			+1.4798105528177163 * y9
			-1.7757726633812596 * y10
			+2.1524517131894054 * y11
			-2.6307743161203847 * y12
			+3.237876081378935  * y13
			-4.008798957897729  * y14
			+4.988727592050507  * y15
			-6.2359094900631336 * y16
		;
		#endif
		#if 0 // Padé approximant at -¼
		//PadeApproximant[Log[1+x],{x,-1/4,{6,6}}]
		#endif
	}
	result += power * 0.6931471805599453;

	return result;

	/*
	from mpmath import mp
	mp.prec = 128
	print(float( -mp.log( mp.mpf(4.0) / mp.mpf(3.0) ) ))
	for k in range(1,16+1,1):
		coeff = -mp.power( -mp.mpf(4.0)/mp.mpf(3.0), k )/k
		print(f"{str(float(coeff))} * y{k}")
	*/
}



/*
Feynman's Algorithm for Logarithms

Compute log₂(v) by adding the logs for a product.  After transforming v so v∈[1,2), we can write:

	v = (1 + 2⁻¹)ᵇ⁰ (1 + 2⁻²)ᵇ¹ (1 + 2⁻³)ᵇ² ⋯ (1 + 2⁻⁵²)ᵇ⁵¹

Where {b₀,b₁,⋯,b₅₂} determine whether a particular factor is present in the product.  When we take
the log, the factors' logs can get summed together.  The table below precomputes these logs.

	log₂(v) = b₀log₂(1+2⁻¹) + b₁log₂(1+2⁻²) + b₂log₂(1+2⁻³) + ⋯ + b₅₂log₂(1+2⁻⁵²)

See also https://cstheory.stackexchange.com/a/3469/

Table generated with:

	from mpmath import mp
	mp.prec = 256

	N = 64

	s = f"static uint64_t const _log2_factors[{N}-1] =\n{{\n\t"
	line_len = 4
	for k in range( 0, N, 1 ):
		fact = mp.mpf(1.0) + mp.power(2.0,-k)
		factlog = mp.log( fact, 2.0 )
		# 63 bits precision after radix point
		# TODO: adjust so 64 bits, since all `fact`<2 means the log is <1
		factstr = factlog.to_fixed(63)
		factstr = f"{factstr:#0{2+16}x}, "
		if k == 0:
			factstr = "/"+"* ( omit k=0 ) *"+"/  "
		if line_len + len(factstr) > 100:
			s = s[:-1] #trailing space
			s += "\n\t"
			line_len = 4
		s += factstr
		line_len += len(factstr)
	s = s[:-2]
	s += "\n};\n"
	print(s)
*/
static uint64_t const _log2_factors[64-1] =
{
	/* ( omit k=0 ) */  0x4ae00d1cfdeb43cf, 0x2934f0979a3715fc, 0x15c01a39fbd6879f,
	0x0b31fb7d64898b3e, 0x05aeb4dd63bf61cc, 0x02dcf2d0b85a4531, 0x016fe50b6ef08517,
	0x00b84e236bd563ba, 0x005c3e0ffc29d593, 0x002e24ca6e87e8a8, 0x001713d62f7957c3,
	0x000b8a476150dfe4, 0x0005c53ac47e94d8, 0x0002e2a32762fa6b, 0x0001715305002e4a,
	0x0000b8a9ded47c11, 0x00005c55067f6e58, 0x00002e2a89050622, 0x0000171545f3d72b,
	0x00000b8aa35640a7, 0x000005c551c23599, 0x000002e2a8e6e01e, 0x000001715474e163,
	0x000000b8aa3acd06, 0x0000005c551d7d98, 0x0000002e2a8ec491, 0x00000017154763ba,
	0x0000000b8aa3b239, 0x00000005c551d933, 0x00000002e2a8ec9f, 0x0000000171547651,
	0x00000000b8aa3b28, 0x000000005c551d94, 0x000000002e2a8eca, 0x0000000017154765,
	0x000000000b8aa3b2, 0x0000000005c551d9, 0x0000000002e2a8ec, 0x0000000001715476,
	0x0000000000b8aa3b, 0x00000000005c551d, 0x00000000002e2a8e, 0x0000000000171547,
	0x00000000000b8aa3, 0x000000000005c551, 0x000000000002e2a8, 0x0000000000017154,
	0x000000000000b8aa, 0x0000000000005c55, 0x0000000000002e2a, 0x0000000000001715,
	0x0000000000000b8a, 0x00000000000005c5, 0x00000000000002e2, 0x0000000000000171,
	0x00000000000000b8, 0x000000000000005c, 0x000000000000002e, 0x0000000000000017,
	0x000000000000000b, 0x0000000000000005, 0x0000000000000002, 0x0000000000000001
};
MOSS_ND double __moss_log2( double val ) MOSS_NOEXCEPT
{
	if ( val<0.0 || __moss_isnan_f64(val) ) return __moss_nand();
	__MOSS_Flt64 bits = __moss_f64_to_bits( val );
	if ( bits.biasexp == 2047/*0b11111111111*/ ) { return val; } /* inf or NaN */

	/* Similar to, but different from, `frexp(⋯)` */
	int charac;
	uint64_t coeff;
	if ( bits.biasexp != 0 ) /* normal */
	{
		charac = bits.biasexp - 1023;
		coeff  = bits.mantissa | (1ull<<52);
	}
	else /* subnormal */
	{
		charac = -1022;
		__moss_assert_impl( bits.mantissa != 0 );
		coeff = bits.mantissa;
		while ( coeff < (1ull<<52) ) { coeff<<=1; --charac; }
	}
	/* `val`=`coeff`*exp2(`charac`), `coeff`∈[1,2) */

	/* Calculation now proceeds in 1p63 fixed-point, for extra precision on the LSBs */
	coeff <<= 11;

	uint64_t mantissa_1p63 = 0;
	uint64_t product = 1ull << 63;
	for ( unsigned k=1; k<sizeof(_log2_factors)/sizeof(uint64_t); ++k )
	{
		uint64_t product_with_factor = product + (product>>k); // multiply by (1+2⁻ᵏ)
		if ( product_with_factor <= coeff )
		{
			product = product_with_factor;
			mantissa_1p63 += _log2_factors[ k - 1 ]; // log₂(1+2⁻ᵏ)
		}
	}
	if ( mantissa_1p63 == 0 ) return charac;

	/* position the mantissa */
	uint64_t mantissa_1p52 = mantissa_1p63;
	int power = -11; // undoes shift
	while ( mantissa_1p52 > 0x001FFFFFFFFFFFFFull ) { mantissa_1p52>>=1; ++power; }
	while ( mantissa_1p52 < 0x0010000000000000ull ) { mantissa_1p52<<=1; --power; }

	bits.biasexp = (unsigned)( power + 1023 );
	bits.mantissa = mantissa_1p52; // note clipping leading 1

	double mantissa = __moss_bits_to_f64( bits );
	double result = (double)charac + mantissa;

	return result;
}
