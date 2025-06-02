#pragma once

#include "c_intelli.h"

#include <inttypes.h>
#include <stdatomic.h>
#include <stdbool.h>

#include <assert.h>
#include <math.h>
#include <string.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>



#define MIN( A, B ) ( (A)<(B) ? (A) : (B) )
#define MAX( A, B ) ( (A)<(B) ? (B) : (A) )



typedef struct { uint32_t x, y; } vec2u;

typedef struct {  int32_t x, y; } vec2i;

__attribute__(( aligned(16) ))
typedef union
{
	struct { float x, y; };
	struct { float a, b; };
	float data[2];
	struct { int:32; int:32; int:32; int:32; };
}
vec2f2x;
static_assert( sizeof(vec2f2x) == 4*sizeof(float) );

__attribute__(( aligned(16) ))
typedef union
{
	struct { float x, y, z; };
	struct { float a, b, c; };
	float data[3];
	struct { int:32; int:32; int:32; int:32; };
}
vec3f1x;
static_assert( sizeof(vec3f1x) == 4*sizeof(float) );

__attribute__(( aligned(16) ))
typedef union
{
	struct { float x, y, z, w; };
	struct { float a, b, c, d; };
	float data[4];
}
vec4f;
static_assert( sizeof(vec4f) == 4*sizeof(float) );

typedef struct { int32_t x, y, z, w; } vec4i;

#define vec2u_( X, Y ) ((vec2u){ X, Y })
#define vec2i_( X, Y ) ((vec2i){ X, Y })
#define vec4f_( X, Y, Z, W ) ((vec4f){{ X, Y, Z, W }})



typedef struct { int32_t x,y; uint32_t w,h; } Rect;
#define Rect_( X,Y, W,H ) ((Rect){ X,Y, W,H })

MOSS_ND bool rect_intersection( Rect r0,Rect r1, Rect* inter );



MOSS_ND_INLINE vec2i vec2i_add( vec2i lhs, vec2i rhs )
{
	return vec2i_( lhs.x+rhs.x, lhs.y+rhs.y );
}

MOSS_ND_INLINE vec4f vec4f_add( vec4f lhs, vec4f rhs )
{
	return vec4f_( lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z, lhs.w+rhs.w );
}
MOSS_ND_INLINE vec4f vec4f_mul( vec4f lhs, vec4f rhs )
{
	return vec4f_( lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z, lhs.w*rhs.w );
}
MOSS_ND_INLINE vec4f vec4f_scale( vec4f vec, float sc )
{
	return vec4f_mul( vec, vec4f_(sc,sc,sc,sc) );
}



MOSS_ND_INLINE int clampi( int val, int lo,int hi )
{
	if ( val < lo ) return lo;
	if ( val > hi ) return hi;
	return val;
}



// https://wiki.osdev.org/Spinlock#Spinlocks_in_C
void mutex_spin_acquire( atomic_flag* mutex );
MOSS_INLINE void mutex_spin_release( atomic_flag* mutex )
{
	atomic_flag_clear_explicit( mutex, memory_order_release );
}
