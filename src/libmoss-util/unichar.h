#pragma once

//#include "stdafx.h"

#include "bits.h"



//Represents a Unicode code point.
//	The internal storage is as UTF-8 bytes, since that is the only legitimate string encoding.
//	Unused bytes are zeroed.

typedef union
{
	struct { uint8_t byte0, byte1, byte2, byte3; };
	uint8_t bytes[4];
	uint32_t packed;
}
unichar;



MOSS_ND_INLINE unichar uni_from_ch( char ch ) { return (unichar){ .packed=ch }; }

MOSS_ND bool uni_extract( char const** str_ptr, unichar* ch );

MOSS_ND bool uni_from_codepoint( uint32_t codepoint, unichar* uni );
MOSS_ND uint32_t uni_to_codepoint( unichar uni );

MOSS_ND_INLINE uint8_t uni_bytecount( unichar uni )
{
	uint8_t ones = clo8( uni.byte0 );
	//Else invalid character, should have been caught earlier
	__moss_assert_impl( ones==0 || ones==2 || ones==3 || ones==4 );

	uint8_t const table[5] = { 1, 0, 2, 3, 4 };
	return table[ ones ];
}
