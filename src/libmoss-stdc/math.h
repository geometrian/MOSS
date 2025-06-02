#pragma once

/*
https://en.cppreference.com/w/c/numeric/math
*/

#include "__arch.h"



#ifdef __MOSS_CPP__
namespace std
{



template< class Tfloat > MOSS_ND_CEXPR
Tfloat __moss_sqrt_nr( Tfloat val, Tfloat curr,Tfloat prev ) noexcept
{
	if ( curr == prev ) return curr;
	return __moss_sqrt_nr( val, Tfloat(0.5)*( curr + val/curr ),curr );
}



extern "C"
{
#endif



/* https://en.wikipedia.org/wiki/Single-precision_floating-point_format */
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
}
__MOSS_Flt32;

/* https://en.wikipedia.org/wiki/Double-precision_floating-point_format */
typedef union
{
	struct
	{
		unsigned long long mantissa : 52;
		unsigned long long biasexp  : 11;
		unsigned long long sign     :  1;
	};
	struct
	{
		unsigned long long absval_as_u64 : 63;
		unsigned long long               :  1;
	};
	double             as_f64;
	unsigned long long as_u64;
}
__MOSS_Flt64;

MOSS_ND_CEXPR __MOSS_Flt32 __moss_f32_to_bits( float  val ) MOSS_NOEXCEPT
{
	__MOSS_Flt32 ret = { .as_u32=__builtin_bit_cast( unsigned int, val ) };
	return ret;
}
MOSS_ND_CEXPR __MOSS_Flt64 __moss_f64_to_bits( double val ) MOSS_NOEXCEPT
{
	__MOSS_Flt64 ret = { .as_u64=__builtin_bit_cast( unsigned long long, val ) };
	return ret;
}

MOSS_ND_CEXPR float  __moss_bits_to_f32( __MOSS_Flt32 bits ) MOSS_NOEXCEPT
{
	return __builtin_bit_cast( float, bits.as_u32 );
}
MOSS_ND_CEXPR double __moss_bits_to_f64( __MOSS_Flt64 bits ) MOSS_NOEXCEPT
{
	return __builtin_bit_cast( double, bits.as_u64 );
}
MOSS_ND_CEXPR float  __moss_u32_to_f32( unsigned int       u32 ) MOSS_NOEXCEPT
{
	return __builtin_bit_cast( float, u32 );
}
MOSS_ND_CEXPR double __moss_u64_to_f64( unsigned long long u64 ) MOSS_NOEXCEPT
{
	return __builtin_bit_cast( double, u64 );
}

MOSS_ND_CEXPR int __moss_isinf_f32( float  val ) MOSS_NOEXCEPT
{
	__MOSS_Flt32 bits = __moss_f32_to_bits( val );
	return bits.biasexp==0xFF && bits.mantissa==0; /* 0b1111'1111, the full 8 bits */
}
MOSS_ND_CEXPR int __moss_isinf_f64( double val ) MOSS_NOEXCEPT
{
	__MOSS_Flt64 bits = __moss_f64_to_bits( val );
	return bits.biasexp==0x7FF && bits.mantissa==0; /* 0b111'1111'1111, the full 11 bits */
}

MOSS_ND_CEXPR int __moss_isnan_f32( float  val ) MOSS_NOEXCEPT
{
	__MOSS_Flt32 bits = __moss_f32_to_bits( val );
	return bits.biasexp==0xFF && bits.mantissa!=0; /* 0b1111'1111, the full 8 bits */
}
MOSS_ND_CEXPR int __moss_isnan_f64( double val ) MOSS_NOEXCEPT
{
	__MOSS_Flt64 bits = __moss_f64_to_bits( val );
	return bits.biasexp==0x7FF && bits.mantissa!=0; /* 0b111'1111'1111, the full 11 bits */
}

MOSS_ND_CEXPR float  __moss_inff(void) MOSS_NOEXCEPT { return __moss_u32_to_f32(0x7f800000u); }
MOSS_ND_CEXPR double __moss_infd(void) MOSS_NOEXCEPT { return __moss_u64_to_f64(0x7ff0000000000000ull); }

MOSS_ND_CEXPR float  __moss_nanf(void) MOSS_NOEXCEPT { return __moss_u32_to_f32(0x7fbfffffu); }
MOSS_ND_CEXPR double __moss_nand(void) MOSS_NOEXCEPT { return __moss_u64_to_f64(0x7ff7ffffffffffffull); }



MOSS_ND_CEXPR float       fabsf( float       val ) MOSS_NOEXCEPT
{
	__MOSS_Flt32 bits = __moss_f32_to_bits( val );
	bits.as_u32 &= 0x7fffffffu;
	return __moss_bits_to_f32( bits );
}
MOSS_ND_CEXPR double      fabs ( double      val ) MOSS_NOEXCEPT
{
	__MOSS_Flt64 bits = __moss_f64_to_bits( val );
	bits.as_u64 &= 0x7fffffffffffffffu;
	return __moss_bits_to_f64( bits );
}
MOSS_ND_CEXPR long double fabsl( long double val ) MOSS_NOEXCEPT { return (long double)fabs( (double)val ); }

/*
Apparently `fma`/`fmaf`(/`fmal`?) become calls, resulting in undefined symbol errors even though
these are `inline`.  Doesn't happen with a different name.  This test works though . . .

	#include <cstdio>
	#include <cstdlib>
	extern "C" float fmaf( float x,float y, float z )
	{
		return x*y + z; //(Not actually FMA)
	}
	int main( int argc, char* argv[] )
	{
		if ( argc != 1+3 ) return -1;
		float x=atof(argv[1]), y=atof(argv[2]), z=atof(argv[3]);
		std::printf( "%f\n", (double)fmaf(x,y,z) );
		return 0;
	}

Also the inline assembly written below doesn't work (the compiler gets the order of the registers
wrong, and anyway it confuses the register scheduler.  Just implement it in C and trust on the
optimizer to do its job.  TODO: better.
*/
#if 0
MOSS_ND_INLINE float       __moss_fmaf( float       x,float       y, float       z ) MOSS_NOEXCEPT
{
	asm(
		"vfmadd213ss   %0,%1, %2\n"
		: "+x"(x)
		: "x"(y), "x"(z) //TODO: why doesn't clang-cl like `z` being in "mx" here?
		:
	);
	return x;
	//return __builtin_fma( x,y, z );
}
MOSS_ND_INLINE double      __moss_fmad( double      x,double      y, double      z ) MOSS_NOEXCEPT
{
	//https://www.felixcloutier.com/x86/vfmadd132sd:vfmadd213sd:vfmadd231sd
	asm(
		"vfmadd213sd   %0,%1, %2\n"
		: "+x"(x)
		: "x"(y), "x"(z) //TODO: why doesn't clang-cl like `z` being in "mx" here?
		:
	);
	return x;
	//return __builtin_fma( x,y, z );
}
#endif
MOSS_ND_INLINE float       __moss_fmaf( float       x,float       y, float       z ) MOSS_NOEXCEPT
{
	return x*y + z;
}
MOSS_ND_INLINE double      __moss_fmad( double      x,double      y, double      z ) MOSS_NOEXCEPT
{
	return x*y + z;
}
MOSS_ND_INLINE long double __moss_fmal( long double x,long double y, long double z ) MOSS_NOEXCEPT
{
	return (long double)__moss_fmad( (double)x,(double)y, (double)z );
}



MOSS_ND double __moss_expm1( double val ) MOSS_NOEXCEPT;
MOSS_ND_INLINE float       expm1f( float       val ) MOSS_NOEXCEPT { return (float)__moss_expm1( (double)val ); }
MOSS_ND_INLINE double      expm1 ( double      val ) MOSS_NOEXCEPT { return __moss_expm1( val ); }
MOSS_ND_INLINE long double expm1l( long double val ) MOSS_NOEXCEPT { return (long double)expm1( (double)val ); }

MOSS_ND_INLINE float       expf( float       val ) MOSS_NOEXCEPT { return 1.0f + expm1f(val); }
MOSS_ND_INLINE double      exp ( double      val ) MOSS_NOEXCEPT { return 1.0  + expm1 (val); }
MOSS_ND_INLINE long double expl( long double val ) MOSS_NOEXCEPT { return 1.0L + expm1l(val); }

MOSS_ND double __moss_log( double val ) MOSS_NOEXCEPT;
MOSS_ND_INLINE float       logf( float       val ) MOSS_NOEXCEPT { return (float)__moss_log( (double)val ); }
MOSS_ND_INLINE double      log ( double      val ) MOSS_NOEXCEPT { return __moss_log( val ); }
MOSS_ND_INLINE long double logl( long double val ) MOSS_NOEXCEPT { return (long double)log( (double)val ); }

MOSS_ND double __moss_log2( double val ) MOSS_NOEXCEPT;
MOSS_ND_INLINE float       log2f( float       val ) MOSS_NOEXCEPT { return (float)__moss_log2( (double)val ); }
MOSS_ND_INLINE double      log2 ( double      val ) MOSS_NOEXCEPT { return __moss_log2( val ); }
MOSS_ND_INLINE long double log2l( long double val ) MOSS_NOEXCEPT { return (long double)log2( (double)val ); }

MOSS_ND_INLINE float       log1pf( float       val ) MOSS_NOEXCEPT { return logf ( val + 1.0f ); }
MOSS_ND_INLINE double      log1p ( double      val ) MOSS_NOEXCEPT { return log  ( val + 1.0  ); }
MOSS_ND_INLINE long double log1pl( long double val ) MOSS_NOEXCEPT { return (long double)log1p( (double)val ); }



MOSS_ND_INLINE float       powf( float       base, float       power ) MOSS_NOEXCEPT
{
	return expf( power * logf(base) );
}
MOSS_ND_INLINE double      pow ( double      base, double      power ) MOSS_NOEXCEPT
{
	return exp( power * log(base) );
}
MOSS_ND_INLINE long double powl( long double base, long double power ) MOSS_NOEXCEPT
{
	return expl( power * logl(base) );
}

MOSS_ND_CEXPR float       sqrtf( float       val ) MOSS_NOEXCEPT
{
	#ifdef __MOSS_CPP__
		if consteval
		{
			if ( val<0.0f || __moss_isnan_f32(val) ) return __moss_nanf();
			if ( __moss_isinf_f32(val) ) return val;
			return __moss_sqrt_nr( val, val,0.0f );
		}
		else
		{
	#endif
			//https://www.felixcloutier.com/x86/sqrtss
			float ret;
			asm( "sqrtss   %0, %1\n" : "=x"(ret) : "x"(val) : );
			return ret;
	#ifdef __MOSS_CPP__
		}
	#endif
}
MOSS_ND_CEXPR double      sqrt ( double      val ) MOSS_NOEXCEPT
{
	#ifdef __MOSS_CPP__
		if consteval
		{
			if ( val<0.0 || __moss_isnan_f64(val) ) return __moss_nand();
			if ( __moss_isinf_f64(val) ) return val;
			return __moss_sqrt_nr( val, val,0.0 );
		}
		else
		{
	#endif
			double ret;
			asm( "sqrtsd   %0, %1\n" : "=x"(ret) : "x"(val) : );
			return ret;
	#ifdef __MOSS_CPP__
		}
	#endif
}
MOSS_ND_CEXPR long double sqrtl( long double val ) MOSS_NOEXCEPT { return (long double)sqrt( (double)val ); }

MOSS_ND double __moss_cbrt( double val ) MOSS_NOEXCEPT;
MOSS_ND_INLINE float       cbrtf( float       val ) MOSS_NOEXCEPT { return (float)__moss_cbrt( (double)val ); }
MOSS_ND_INLINE double      cbrt ( double      val ) MOSS_NOEXCEPT { return __moss_cbrt( val ); }
MOSS_ND_INLINE long double cbrtl( long double val ) MOSS_NOEXCEPT { return (long double)cbrt( (double)val ); }



/*
TODO: better
Renamed, same as for fma
*/
MOSS_ND_CEXPR float       __moss_roundf( float       val ) MOSS_NOEXCEPT
{
	return (float)(long long)(val+0.5f);
}
MOSS_ND_CEXPR double      __moss_roundd( double      val ) MOSS_NOEXCEPT
{
	return (double)(long long)(val+0.5);
}
MOSS_ND_CEXPR long double __moss_roundl( long double val ) MOSS_NOEXCEPT { return (long double)__moss_roundd( (double)val ); }

MOSS_ND_CEXPR float       roundf( float       arg ) MOSS_NOEXCEPT { return __moss_roundf(arg); }
MOSS_ND_CEXPR double      round ( double      arg ) MOSS_NOEXCEPT { return __moss_roundd(arg); }
MOSS_ND_CEXPR long double roundl( long double arg ) MOSS_NOEXCEPT { return __moss_roundl(arg); }

MOSS_ND_CEXPR long       lroundf( float       val ) MOSS_NOEXCEPT { return (long)__moss_roundf( val ); }
MOSS_ND_CEXPR long       lround ( double      val ) MOSS_NOEXCEPT { return (long)__moss_roundd( val ); }
MOSS_ND_CEXPR long       lroundl( long double val ) MOSS_NOEXCEPT { return lround( (double)val ); }

MOSS_ND_CEXPR long long llroundf( float       val ) MOSS_NOEXCEPT { return (long long)__moss_roundf( val ); }
MOSS_ND_CEXPR long long llround ( double      val ) MOSS_NOEXCEPT { return (long long)__moss_roundd( val ); }
MOSS_ND_CEXPR long long llroundl( long double val ) MOSS_NOEXCEPT { return llround( (double)val ); }

/* Returns `frac` ∈ [½,1) and `power` such that `val`=`frac`∙(2^`*power`).  Similarly for negative. */
MOSS_ND        float       frexpf( float       val, int* power ) MOSS_NOEXCEPT;
MOSS_ND        double      frexp ( double      val, int* power ) MOSS_NOEXCEPT;
MOSS_ND_INLINE long double frexpl( long double val, int* power ) MOSS_NOEXCEPT
{
	return (long double)frexp( (double)val, power );
}



#ifdef __MOSS_C__
#define isinf( VAL )\
	( sizeof(VAL)==4 ? __moss_isinf_f32((float)VAL) : __moss_isinf_f64((double)VAL) )
#define isnan( VAL )\
	( sizeof(VAL)==4 ? __moss_isnan_f32((float)VAL) : __moss_isnan_f64((double)VAL) )
#endif



#ifdef __MOSS_CPP__
}



}
#endif
