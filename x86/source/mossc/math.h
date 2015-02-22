#pragma once

#include "math/trig.h"


/*MOSS_DECL_FP2(exp)
MOSS_DECL_FP2(pow)

MOSS_DECL_FP1(sqrt)
MOSS_DECL_FP1(cbrt)
MOSS_DECL_FP2(hypot)
MOSS_DECL_FP2(hypotsq)

MOSS_DECL_FP1(ceil)
MOSS_DECL_FP1(floor)*/

template <typename type> inline type abs(type x) {
	return x<0 ? -x : x;
}