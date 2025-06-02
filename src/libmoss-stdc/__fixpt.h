#pragma once

#include <stdint.h>



typedef struct
{
	uint8_t is_neg;

	uint16_t decpt_ind;

	uint16_t digits_count;
	uint8_t  digits[1076];
}
_MOSS_FixPtBCD1076;

void __moss_fixpt_bcd1076_zero( _MOSS_FixPtBCD1076* bcd );

void __moss_fixpt_bcd1076_truncate( _MOSS_FixPtBCD1076* bcd, uint16_t num_digits_to_remove );

void __moss_fixpt_bcd1076_mul_u60( _MOSS_FixPtBCD1076* bcd, uint64_t val60 );
void __moss_fixpt_bcd1076_div_2( _MOSS_FixPtBCD1076* bcd );

void __moss_fixpt_bcd1076_append_frac_zero( _MOSS_FixPtBCD1076* bcd );

void __moss_fixpt_bcd1076_round_to( _MOSS_FixPtBCD1076* bcd, uint16_t max_frac_digits );

/* Create BCD from finite (and non-NaN) floating-point value */
void __moss_fixpt_bcd1076_from_f64( _MOSS_FixPtBCD1076* bcd, double finite_val );
