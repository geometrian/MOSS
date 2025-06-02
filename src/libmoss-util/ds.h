#pragma once

#include "stdafx.h"



void mutex_spin_acquire( atomic_flag* mutex );
MOSS_INLINE void mutex_spin_release( atomic_flag* mutex );

__attribute__(( format(printf,1,2) ))
int klogf( char const* fmt_cstr,... );



// `TinyRingBuffer`
#if 1

#define MOSS_DS_TRB_MAXCOUNT 31

struct TinyRingBuffer
{
	union
	{
		struct { uint8_t _start, _end; }; // valid range [`start`,`end`)
		atomic_ushort _packed_range;
	};
	uint8_t _data[MOSS_DS_TRB_MAXCOUNT+1];
};
static_assert( sizeof(struct TinyRingBuffer) == 2+MOSS_DS_TRB_MAXCOUNT+1 );

MOSS_ND_INLINE struct TinyRingBuffer ds_trb_init(void)
{
	struct TinyRingBuffer trb;
	trb._start = 0;
	trb._end   = 0;
	return trb;
}

MOSS_INLINE void ds_trb_clear( struct TinyRingBuffer* trb )
{
	trb->_start = 0;
	trb->_end   = 0;
}

MOSS_INLINE uint8_t ds_trb_count( struct TinyRingBuffer const* trb )
{
	uint16_t rng = trb->_packed_range; // Note atomic load; this is a safe function to call
	uint8_t s=(uint8_t)rng, e=rng>>8;
	//uint8_t s,e; asm( "mov %0,ah\nmov %1,al" : "=r"(e),"=r"(s) : "a"(rng) : );
	//uint8_t s,e; memcpy(&s,&rng,1); memcpy(&e,(uint8_t*)&rng+1,1);

	uint8_t count = e - s;
	//if ( s >= e ) count+=MOSS_DS_TRB_MAXCOUNT+1;
	static_assert( MOSS_DS_TRB_MAXCOUNT>0 && (MOSS_DS_TRB_MAXCOUNT&(MOSS_DS_TRB_MAXCOUNT+1))==0 );
	count &= MOSS_DS_TRB_MAXCOUNT; // if power of 2 minus 1, can do this instead

	//klogf( "{(%u,%u) len %u}", s,e, count );

	return count;
}

MOSS_INLINE uint8_t ds_trb_get( struct TinyRingBuffer const* trb, uint8_t ind )
{
	__moss_assert( ind<ds_trb_count(trb), "Index %u outside of buffer of size %u!",
		ind,ds_trb_count(trb)
	);
	return trb->_data[ (trb->_start+ind) % (MOSS_DS_TRB_MAXCOUNT+1) ];
}

MOSS_INLINE bool ds_trb_is_empty( struct TinyRingBuffer const* trb )
{
	bool ret = trb->_start == trb->_end;
	__moss_assert_impl( ret == (ds_trb_count(trb)==0) );
	return ret;
}

MOSS_INLINE uint8_t ds_trb_peek_front( struct TinyRingBuffer const* trb )
{
	__moss_assert( !ds_trb_is_empty(trb), "Buffer empty!" );
	return trb->_data[ trb->_start ];
}

MOSS_INLINE void ds_trb_pop_front   ( struct TinyRingBuffer* trb, uint8_t* val )
{
	#ifdef MOSS_DEBUG
	//klogf( "{pop %u(%u,%u) -> ", ds_trb_count(trb), trb->start,trb->end );
	uint8_t count_before = ds_trb_count(trb);
	__moss_assert( count_before>0, "Buffer empty!" );
	#endif

	*val = trb->_data[ trb->_start ];
	trb->_start = (trb->_start+1) % (MOSS_DS_TRB_MAXCOUNT+1);

	#ifdef MOSS_DEBUG
	uint8_t count_after = ds_trb_count(trb);
	__moss_assert( count_before-1==count_after, "Invalid buffer pop %u -> %u",
		count_before, count_after
	);
	//klogf( "%02x,%u(%u,%u)}", *val, ds_trb_count(trb), trb->start,trb->end );
	#endif
}
MOSS_INLINE bool ds_trb_trypop_front( struct TinyRingBuffer* trb, uint8_t* val )
{
	if ( ds_trb_is_empty(trb) ) return false;
	ds_trb_pop_front( trb, val );
	return true;
}

MOSS_INLINE void ds_trb_push_back   ( struct TinyRingBuffer* trb, uint8_t val )
{
	#ifdef MOSS_DEBUG
	//klogf( "{push %02x,%u(%u,%u) -> ", val, ds_trb_count(trb), trb->start,trb->end );
	uint8_t count_before = ds_trb_count(trb);
	__moss_assert( count_before<MOSS_DS_TRB_MAXCOUNT, "Buffer full!" );
	#endif

	trb->_data[ trb->_end ] = val;
	trb->_end = (trb->_end+1) % (MOSS_DS_TRB_MAXCOUNT+1);

	#ifdef MOSS_DEBUG
	uint8_t count_after = ds_trb_count(trb);
	__moss_assert( count_before+1==count_after, "Invalid buffer push %u -> %u",
		count_before, count_after
	);
	//klogf( "%u(%u,%u)}", ds_trb_count(trb), trb->start,trb->end );
	#endif
}
MOSS_INLINE bool ds_trb_trypush_back( struct TinyRingBuffer* trb, uint8_t val )
{
	if ( ds_trb_count(trb) == MOSS_DS_TRB_MAXCOUNT ) return false;
	ds_trb_push_back( trb, val );
	return true;
}

#endif



// `TinyAtomicQueue`
#if 1

struct TinyAtomicQueue
{
	atomic_flag lock;
	struct TinyRingBuffer buffer;
};

MOSS_ND_INLINE struct TinyAtomicQueue ds_taq_init(void)
{
	struct TinyAtomicQueue taq;
	memset(           &taq  , 0x00, 2 );
	memset( (uint8_t*)&taq+2, 0x01, 1 );
	return taq;
}

MOSS_INLINE void ds_taq_clear( struct TinyAtomicQueue* taq )
{
	mutex_spin_acquire( &taq->lock );
	ds_trb_clear( &taq->buffer );
	mutex_spin_release( &taq->lock );
}

MOSS_INLINE void ds_taq_push   ( struct TinyAtomicQueue* taq, uint8_t val )
{
	mutex_spin_acquire( &taq->lock );
	ds_trb_push_back( &taq->buffer, val );
	mutex_spin_release( &taq->lock );
}
MOSS_INLINE bool ds_taq_trypush( struct TinyAtomicQueue* taq, uint8_t val )
{
	if ( ds_trb_count(&taq->buffer) == MOSS_DS_TRB_MAXCOUNT ) return false;

	mutex_spin_acquire( &taq->lock );
	bool ret = ds_trb_trypush_back( &taq->buffer, val );
	mutex_spin_release( &taq->lock );
	return ret;
}
MOSS_INLINE void ds_taq_pop   ( struct TinyAtomicQueue* taq, uint8_t* val )
{
	mutex_spin_acquire( &taq->lock );
	ds_trb_pop_front( &taq->buffer, val );
	mutex_spin_release( &taq->lock );
}
MOSS_INLINE bool ds_taq_trypop( struct TinyAtomicQueue* taq, uint8_t* val )
{
	if ( ds_trb_count(&taq->buffer) == 0 ) return false;

	mutex_spin_acquire( &taq->lock );
	bool ret = ds_trb_trypop_front( &taq->buffer, val );
	mutex_spin_release( &taq->lock );
	return ret;
}

#endif



// `TinyBuffer` (incomplete)
#if 0

// Stores up to 24 bytes of data
struct TinyBuffer
{
	uint8_t  count;
	uint64_t packed[3];
};

MOSS_INLINE void ds_tb_init( struct TinyBuffer* tb ) { tb->count = 0; }
MOSS_INLINE void ds_tb_clear( struct TinyBuffer* tb ) { tb->count = 0; }
MOSS_INLINE void ds_tb_push_front( struct TinyBuffer* tb, uint8_t val )
{
	__moss_assert( tb->count<24, "Buffer full!" );
	#if 0
		tb->packed[2] = (tb->packed[2]<<8) | (tb->packed[1]>>48);
		tb->packed[1] = (tb->packed[1]<<8) | (tb->packed[0]>>48);
		tb->packed[0] <<= 8;
		tb->packed[0] |= val;
	#else
		asm(
			"shld   %2, %1, 8\n"
			"shld   %1, %0, 8\n"
			"shl    %0, 8\n"
			"or     %b0, %3\n"
			: "+rm"(tb->packed[0]), "+rm"(tb->packed[1]), "+rm"(tb->packed[2])
			: "r"(val)
			:
		);
	#endif
	--tb->count;
}

// https://www.felixcloutier.com/x86/shld

#endif
