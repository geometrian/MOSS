#pragma once

#include "stdafx.h"



/*
Need to check for zero, since in that case the intrinsic gives an indeterminate result.  It can be
done with w.g. a very clever `cmov` after a `bsr`, but it's probably faster to do a branch, which
can be predicted and also serializes less -> better for out-of-order processors.
*/

MOSS_ND_INLINE uint8_t clz8 ( uint8_t  val )
{
	if ( val == 0 ) return 8;
	return (uint8_t)__builtin_clzs((uint16_t)val) - 8;
}
MOSS_ND_INLINE uint8_t clz16( uint16_t val )
{
	if ( val == 0 ) return 16;
	return (uint8_t)__builtin_clzs(val);
}
MOSS_ND_INLINE uint8_t clz32( uint32_t val )
{
	if ( val == 0 ) return 32;
	return (uint8_t)__builtin_clz(val);
}
MOSS_ND_INLINE uint8_t clz64( uint64_t val )
{
	if ( val == 0 ) return 64;
	return (uint8_t)__builtin_clzll(val);
}

MOSS_ND_INLINE uint8_t ctz8 ( uint8_t  val )
{
	if ( val == 0 ) return 8;
	return (uint8_t)__builtin_ctzs((uint16_t)val) - 8;
}
MOSS_ND_INLINE uint8_t ctz16( uint16_t val )
{
	if ( val == 0 ) return 16;
	return (uint8_t)__builtin_ctzs(val);
}
MOSS_ND_INLINE uint8_t ctz32( uint32_t val )
{
	if ( val == 0 ) return 32;
	return (uint8_t)__builtin_ctz(val);
}
MOSS_ND_INLINE uint8_t ctz64( uint64_t val )
{
	if ( val == 0 ) return 64;
	return (uint8_t)__builtin_ctzll(val);
}

MOSS_ND_INLINE uint8_t clo8 ( uint8_t  val ) { return clz8 ((uint8_t )~val); }
MOSS_ND_INLINE uint8_t clo16( uint16_t val ) { return clz16((uint16_t)~val); }
MOSS_ND_INLINE uint8_t clo32( uint32_t val ) { return clz32(          ~val); }
MOSS_ND_INLINE uint8_t clo64( uint64_t val ) { return clz64(          ~val); }

MOSS_ND_INLINE uint8_t cto8 ( uint8_t  val ) { return ctz8 ((uint8_t )~val); }
MOSS_ND_INLINE uint8_t cto16( uint16_t val ) { return ctz16((uint16_t)~val); }
MOSS_ND_INLINE uint8_t cto32( uint32_t val ) { return ctz32(          ~val); }
MOSS_ND_INLINE uint8_t cto64( uint64_t val ) { return ctz64(          ~val); }

MOSS_ND_INLINE uint8_t popcnt8 ( uint8_t val )
{
	return (uint8_t)__builtin_popcount( (uint32_t)val );
}
MOSS_ND_INLINE uint8_t popcnt16( uint16_t val )
{
	return (uint8_t)__builtin_popcount( (uint32_t)val );
}
MOSS_ND_INLINE uint8_t popcnt32( uint32_t val )
{
	return (uint8_t)__builtin_popcount( val );
}
MOSS_ND_INLINE uint8_t popcnt64( uint64_t val )
{
	return (uint8_t)__builtin_popcountll( val );
}

MOSS_ND_INLINE uint8_t  rol8 ( uint8_t  val, uint8_t amt )
{
	return (uint8_t )( (uint8_t )(val<<amt) | (uint8_t )(val>>( 8-amt)) );
}
MOSS_ND_INLINE uint16_t rol16( uint16_t val, uint8_t amt )
{
	return (uint16_t)( (uint16_t)(val<<amt) | (uint16_t)(val>>(16-amt)) );
}
MOSS_ND_INLINE uint32_t rol32( uint32_t val, uint8_t amt )
{
	return (uint32_t)( (uint32_t)(val<<amt) | (uint32_t)(val>>(32-amt)) );
}
MOSS_ND_INLINE uint64_t rol64( uint64_t val, uint8_t amt )
{
	return (uint64_t)( (uint64_t)(val<<amt) | (uint64_t)(val>>(64-amt)) );
}

MOSS_ND_INLINE uint8_t  ror8 ( uint8_t  val, uint8_t amt )
{
	return (uint8_t )( (uint8_t )(val>>amt) | (uint8_t )(val<<( 8-amt)) );
}
MOSS_ND_INLINE uint16_t ror16( uint16_t val, uint8_t amt )
{
	return (uint16_t)( (uint16_t)(val>>amt) | (uint16_t)(val<<(16-amt)) );
}
MOSS_ND_INLINE uint32_t ror32( uint32_t val, uint8_t amt )
{
	return (uint32_t)( (uint32_t)(val>>amt) | (uint32_t)(val<<(32-amt)) );
}
MOSS_ND_INLINE uint64_t ror64( uint64_t val, uint8_t amt )
{
	return (uint64_t)( (uint64_t)(val>>amt) | (uint64_t)(val<<(64-amt)) );
}
