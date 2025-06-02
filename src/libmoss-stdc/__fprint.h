#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "math.h" //`__MOSS_Flt64`
#include "stdio.h" //`FILE`



/*
Formats `val` as a decimal integer.

`prec` specifies the minimum number of digits to appear (-1 results in default 1).  0 can result in
no output if the input is zero.

If `septicks` is true, output is separated with '\'' by threes, e.g. "299'792'458".

If it represents a buffer, `stream` should be large enough to hold the output.  Roughly, 20+1 is
good without ticks, 26+1 is good with ticks.  To also account for `prec`, see something like this
pseudocode:

	int num_digits = MAX( 20, prec );
	buf_sz = num_digits + 1;
	if (septicks&&num_digits>3) buf_sz+=(num_digits-1)/3;

The number of characters written, or EOF, is returned.
*/
int __moss_fprint_dec_u( FILE* stream, uint64_t val, int prec, bool septicks );

int __moss_fprint_fp_dec( FILE* stream, double val, char specifier,int prec,bool always_pt );

//int __moss_fprint_fp_exp( FILE* stream, double val, char specifier,int prec,bool always_pt );

/*
Formats `val` as hex exponential floating-point.

`specifier` must be 'a' or 'A' to use lowercase or uppercase letters in formatting.

`prec` specifies the number of digits after the radix point (-1 strips all but one trailing zero).
If `always_pt` is `true`, then always prints the radix point, even if `prec` is zero.

If it represents a buffer, `stream` should be large enough to hold the output.  24+1 is good, since
the longest default output is e.g. "-0x1.123456789abcdp-1023".  In general:

	buf_sz = 11 + MAX(13,prec) + 1
*/
int __moss_fprint_fp_hex( FILE* stream, double val, char specifier,int prec,bool always_pt );

int __moss_fprint_fp( FILE* stream, double val, char specifier,int prec,bool always_pt );
