#include "int_arith.h"


int abs(int n) {
	return n<0 ? -n : n;
}
long int labs(long int n) {
	return n<0l ? -n : n;
}
long long int llabs(long long int n) {
	return n<0ll ? -n : n;
}

#define DIV_BODY(TYPE)\
	TYPE result;\
	result.quot = numer / denom;\
	result. rem = numer % denom;\
	return result;
div_t div(int numer, int denom) {
	DIV_BODY(div_t)
}
ldiv_t ldiv(long int numer, long int denom) {
	DIV_BODY(ldiv_t)
}
lldiv_t lldiv(long long int numer, long long int denom) {
	DIV_BODY(lldiv_t)
}