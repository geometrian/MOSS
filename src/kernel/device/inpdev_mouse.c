#include "inpdev.h"



void inpdev_mouse_moved( DEVID dev_id, vec2i rel )
{
	struct InpDev_Mouse* mouse = inpdev_mouse_get( dev_id );

	mouse->rel = rel;

	mouse->pos = vec2i_add( mouse->pos, mouse->rel );

	mouse->pos.x = clampi( mouse->pos.x, 0,1024 ); // TODO: clamp
	mouse->pos.y = clampi( mouse->pos.y, 0, 768 );
}
void inpdev_mouse_scroll( DEVID dev_id, int rel_scroll )
{
	struct InpDev_Mouse* mouse = inpdev_mouse_get( dev_id );
	mouse->rel_scroll = rel_scroll;
}
