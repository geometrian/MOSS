#pragma once

#include "../includes.h"

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

template <typename type_num> inline type_num min(type_num num0, type_num num1) {
	return num0<num1 ? num0 : num1;
}
template <typename type_num> inline type_num max(type_num num0, type_num num1) {
	return num0<num1 ? num1 : num0;
}