#include "trig.h"


namespace MOSSC {


#if 0
float cos(float x) {
	//Taylor series is (-1)^(k)*(x^(2k))/((2k)!)
	//1 - x*x/(2!) + x*x*x*x/(4!) - ...

	float result = 1.0f;
	float last_result;

	float xsq = x*x;

	float numerator = -xsq;
	uint64_t denominator = 2;
	uint64_t k_times_2 = 2;

	do {
		result += numerator / (float)(denominator);
	} while (last_result!=result);
	return result;
}
float sin(float x) {
	//Taylor series is (-1)^(k)*(x^(1+2k))/((1+2k)!)
	//x - x^3/(3!) + x^5/(5!) - ...

	float result = x;
	float last_result;

	float nxsq = -x*x;

	float numerator = nxsq * x;
	uint64_t denominator = 6;
	uint64_t k_times_2_plus_1 = 3;

	int i = 0;
	LOOP:
		result += numerator / (float)(denominator);
		if (result!=last_result) {
			numerator *= nxsq;
			denominator *= ++k_times_2_plus_1;
			denominator *= ++k_times_2_plus_1;
			last_result = result;
			if (++i!=10) { //This is really precise
				goto LOOP;
			}
		}
	return result;
}
/*MOSS_DECL_FP1(sin)
MOSS_DECL_FP1(tan)
MOSS_DECL_FP1(acos)
MOSS_DECL_FP1(asin)
MOSS_DECL_FP1(atan)
MOSS_DECL_FP2(atan2)

MOSS_DECL_FP1(cosh)
MOSS_DECL_FP1(sinh)
MOSS_DECL_FP1(tanh)
MOSS_DECL_FP1(acosh)
MOSS_DECL_FP1(asinh)
MOSS_DECL_FP1(atanh)*/
#endif


}