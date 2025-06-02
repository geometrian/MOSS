#include "stdafx.h"



MOSS_ND bool rect_intersection( Rect r0,Rect r1, Rect* inter )
{
	vec2i lo0=vec2i_(r0.x,r0.y), hi0=vec2i_(r0.x+r0.w,r0.y+r0.h);
	vec2i lo1=vec2i_(r1.x,r1.y), hi1=vec2i_(r1.x+r1.w,r1.y+r1.h);

	int xlo = MAX( lo0.x, lo1.x );
	int xhi = MIN( hi0.x, hi1.x );
	if ( xhi <= xlo ) return false;

	int ylo = MAX( lo0.y, lo1.y );
	int yhi = MIN( hi0.y, hi1.y );
	if ( yhi <= ylo ) return false;

	*inter = Rect_( xlo,ylo, xhi-xlo,yhi-ylo );

	return true;
}



void mutex_spin_acquire( atomic_flag* mutex )
{
	while (atomic_flag_test_and_set_explicit( mutex, memory_order_acquire ))
	{
		__builtin_ia32_pause();
	}
}
