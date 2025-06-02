#include "unichar.h"

#include "bits.h"



static_assert( sizeof(char) == 1 );

// TODO: check endian little

static_assert( sizeof(unichar) == 4 );



MOSS_ND bool uni_extract( char const** str_ptr, unichar* ch )
{
	__moss_assert( str_ptr!=NULL && *str_ptr!=NULL && **str_ptr!='\0',
		"`str_ptr` must be a non null pointer to a nonempty string!"
	);
	char const* str = *str_ptr;

	ch->packed = 0u;

	//TODO: optimize with `Bit::clz(...)`?
	ch->byte0 = *str;
	if ( (ch->byte0&0x80) == 0x00 )
	{
		// One byte
		++*str_ptr;
	}
	else
	{
		//TODO: count leading ones
		size_t num_add;
		if      ( (ch->byte0&0xe0) == 0xc0 ) num_add=1; // Two bytes
		else if ( (ch->byte0&0xf0) == 0xe0 ) num_add=2; // Three bytes
		else if ( (ch->byte0&0xf8) == 0xf0 ) num_add=3; // Four bytes
		else return false;

		++str;
		for ( size_t k=0; k<num_add; ++k,++str )
		{
			if ( *str!='\0' && (*str&0xc0)==0x80 )
			{
				ch->bytes[1+k] = *str;
			}
			else return false;
		}

		*str_ptr = str;
	}

	return true;
}

MOSS_ND bool uni_from_codepoint( uint32_t codepoint, unichar* uni )
{
	if        ( codepoint <= 0x0000007fu )
	{
		uni->packed = codepoint;
	}
	else
	{
		uint32_t content0 = codepoint & 0x0000003fu;
		uint32_t content1 = codepoint >> 6; //May need to be masked
		if ( codepoint <= 0x000007ffu )
		{
			uni->packed = content1 | (content0>>8) | 0x000080c0u;
		}
		else
		{
			content1 &= 0x0000003fu;
			uint32_t content2 = codepoint >> 12; //May need to be masked
			if ( codepoint <= 0x0000ffffu )
			{
				uni->packed = content2 | (content1>>8) | (content0>>16) | 0x008080e0u;
			}
			else
			{
				if ( codepoint > 0x001fffffu ) return false;
				content2 &= 0x0000003fu;
				uint32_t content3 = codepoint >> 18;
				uni->packed = content3 | (content2>>8) | (content1>>16) | (content0>>24) | 0x808080f0u;
			}
		}
	}
	return true;
}
MOSS_ND uint32_t uni_to_codepoint( unichar uni )
{
	uint8_t code = clz8( (uint8_t)~uni.byte0 );
	__moss_assert_impl( code==0 || code==2 || code==3 || code==4 );

	switch (code)
	{
		case 0: //One byte
			return (uint32_t)uni.byte0;
		case 2: //Two bytes
			uni.packed &= 0x00001f3fu; //0b00000000'00000000'00011111'00111111u;
			return ((uint32_t)uni.byte0<< 6) | (uint32_t)uni.byte1;
		case 3: //Three bytes
			uni.packed &= 0x000f3f3fu; //0b00000000'00001111'00111111'00111111u;
			return ((uint32_t)uni.byte0<<12) | ((uint32_t)uni.byte1<< 6) |  (uint32_t)uni.byte2;
		case 4: //Four bytes
			uni.packed &= 0x073f3f3fu; //0b00000111'00111111'00111111'00111111u;
			return ((uint32_t)uni.byte0<<18) | ((uint32_t)uni.byte1<<12) | ((uint32_t)uni.byte2<<6) | (uint32_t)uni.byte3;
		__moss_switch_default_unnecessary;
	}
}
