#include "../libmoss-stdc/__env.h"
#include "../libmoss-stdcpp/cmath"
#include "../libmoss-stdcpp/cstdio"



#if 0

#include <cmath>
#include <cstdio>

typedef union
{
	struct
	{
		unsigned mantissa : 23;
		unsigned biasexp  :  8;
		unsigned sign     :  1;
	};
	struct
	{
		unsigned absval_as_u32 : 31;
		unsigned               :  1;
	};
	float    as_f32;
	unsigned as_u32;
} __MOSS_Flt32;

__MOSS_Flt32 __moss_f32_to_bits( float  val )
{
	__MOSS_Flt32 ret = { .as_u32=__builtin_bit_cast( unsigned int, val ) };
	return ret;
}
float  __moss_bits_to_f32( __MOSS_Flt32 bits )
{
	return __builtin_bit_cast( float, bits.as_u32 );
}

float gt_frexpf( float  val, int* power )
{
	return frexpf( val, power );
}
float my_frexpf( float  val, int* power )
{
	if ( val == 0.0f ) { *power=0; return val; }
	__MOSS_Flt32 bits = __moss_f32_to_bits( val );
	if ( bits.biasexp == 0b11111111 ) { *power=0; return val; } /* inf or NaN */

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

void test_val( float val )
{
	int pow1, pow2;
	float val1 = gt_frexpf( val, &pow1 );
	float val2 = my_frexpf( val, &pow2 );
	printf( "%f:\n  %f %d\n  %f %d\n", (double)val, (double)val1,pow1, (double)val2,pow2 );
}

int main()
{
	test_val( -10.0f );
	test_val( - 3.1f );
	test_val( - 0.5f );
	test_val( - 0.0f );
	test_val( - 1.0e-40f );
	test_val(   0.0f );
	test_val(   1.0e-40f );
	test_val(   0.5f );
	test_val(   3.1f );
	test_val(  10.0f );
}

#endif


/*
import struct
from mpmath import mp
mp.prec = 256

VALS = [ 1.0e-40, 1.0e-6, 0.25, 0.5, 1.0, 3.1, 10.0, 100.0 ]
FN,FNNAME = mp.exp,"std::exp"
#FN,FNNAME = lambda x:mp.log(x,2),"std::log2"
#FN,FNNAME = mp.log,"std::log"

for val in VALS:
	ret = float(FN( val ))
	#s = struct.pack("d",ret).hex()
	s = f"{ret:+}"
	while len(s)<19: s+=" "
	s += f", {FNNAME} );"
	print(s)
*/

template< class TypeFn >
void test_fn1( double arg, double gt, TypeFn const& fn ) noexcept
{
	double val = fn( arg );
	std::printf( "fn(%f):\n  gt:   %#llb (%f)\n  ours: %#llb (%f)\n\n", arg,
		std::__moss_f64_to_bits(gt ).as_u64, gt,
		std::__moss_f64_to_bits(val).as_u64, val
	);
}

static void _test_exp()
{
	//TODO: 0 and neg
	test_fn1(   1.0e-40, +    1.0                   , std::exp );
	test_fn1(   1.0e-6 , +    1.0000010000005       , std::exp );
	test_fn1(   0.25   , +    1.2840254166877414    , std::exp );
	test_fn1(   0.5    , +    1.6487212707001282    , std::exp );
	test_fn1(   1.0    , +    2.718281828459045     , std::exp );
	test_fn1(   3.1    , +   22.197951281441636     , std::exp );
	test_fn1(  10.0    , +22026.465794806718        , std::exp );
	test_fn1( 100.0    , +    2.6881171418161356e+43, std::exp );
}

static void _test_log2()
{
	test_fn1(   1.0e-40, -132.8771237954945 , std::log2 );
	test_fn1(   1.0e-6 , -19.931568569324174, std::log2 );
	test_fn1(   0.25   , -2.0               , std::log2 );
	test_fn1(   0.5    , -1.0               , std::log2 );
	test_fn1(   1.0    , +0.0               , std::log2 );
	test_fn1(   3.1    , +1.632268215499513 , std::log2 );
	test_fn1(  10.0    , +3.321928094887362 , std::log2 );
	test_fn1( 100.0    , +6.643856189774724 , std::log2 );
}

static void _test_log()
{
	test_fn1(   1.0e-40, -92.10340371976183 , std::log );
	test_fn1(   1.0e-6 , -13.815510557964274, std::log );
	test_fn1(   0.25   , -1.3862943611198906, std::log );
	test_fn1(   0.5    , -0.6931471805599453, std::log );
	test_fn1(   1.0    , +0.0               , std::log );
	test_fn1(   3.1    , +1.1314021114911006, std::log );
	test_fn1(  10.0    , +2.302585092994046 , std::log );
	test_fn1( 100.0    , +4.605170185988092 , std::log );
}



int main( int /*arc*/, char* /*argv*/[] )
{
	//_test_exp();
	//_test_log2();
	_test_log();

	(void)std::getchar();

	return 0;
}
