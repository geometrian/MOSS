#include "math.h"



MOSS_ND double __moss_expm1( double val ) MOSS_NOEXCEPT
{
	if ( __moss_isnan_f64(val) ) return __moss_nand();
	if ( __moss_isinf_f64(val) ) return val<0.0 ? -1.0 : val;

	/*
	See Matters Computational §32.2.3
	*/

	unsigned n = 0;
	while ( fabs(val) >= 1.0/4.0 )
	{
		val *= 1.0/16.0;
		n += 4;
	}

	double E;
	{
		double v1 = val;

		double v2 = v1 * v1;

		double v3 = v2 * v1;
		double v4 = v2 * v2;

		#if 1
		double v5 = v3 * v2;
		double v6 = v3 * v3;
		double v7 = v4 * v3;
		double v8 = v4 * v4;
		#endif

		E =
			                  v1
			+ (1.0/    2.0) * v2
			+ (1.0/    6.0) * v3
			+ (1.0/   24.0) * v4
			#if 1
			+ (1.0/  120.0) * v5
			+ (1.0/  720.0) * v6
			+ (1.0/ 5040.0) * v7
			+ (1.0/40320.0) * v8
			#endif
		;
	}

	//for ( unsigned k=0; k<n; ++k ) E=2.0*E+E*E;
	for ( unsigned k=0; k<n; k+=4 )
	{
		E = 2.0*E + E*E;
		E = 2.0*E + E*E;
		E = 2.0*E + E*E;
		E = 2.0*E + E*E;
	}

	return E;
}
