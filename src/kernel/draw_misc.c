#include "draw.h"



static_assert( sizeof(A_sRGB) == 4 );

static_assert( sizeof(sRGB_A) == 4 );

static_assert( sizeof(sBGR_A) == 4 );



void draw_img_init( struct Image* img, COLOR color_type, vec2u res, void* data )
{
	img->color_type = color_type;
	img->res = res;
	img->scanline_sz = img->res.x * 4; // TODO: from `color_type`
	img->data = data;
}

void draw_grfx_init( struct Graphics* grfx, struct Image* fbo, struct BitmapFont const* font )
{
	grfx->fbo = fbo;

	draw_set_color( grfx, sRGB_A_( 255,255,255, 255 ) );
	grfx->font = font;

	grfx->text_pos = vec2i_(0,0);
	grfx->text_x_wrap = 0;
}

void draw_set_color( struct Graphics* grfx, sRGB_A color )
{
	grfx->color_native = color_srgba_to_sbgra( color ).packed; // `sBGR_A`
}

bool draw_at( struct Graphics const* grfx, vec2i pos )
{
	if ( pos.x<0 || pos.x>=grfx->fbo->w ) return false;
	if ( pos.y<0 || pos.y>=grfx->fbo->h ) return false;

	draw_at_unchecked( grfx, pos );

	return true;
}

void draw_fill( struct Graphics const* grfx )
{
	for ( uint32_t j=0; j<grfx->fbo->h; ++j )
	for ( uint32_t i=0; i<grfx->fbo->w; ++i )
	{
		draw_at_unchecked( grfx, vec2i_(i,j) );
	}
}

void draw_line_h( struct Graphics const* grfx, int x0,unsigned xlen, int y )
{
	if ( y<0 || y>=grfx->fbo->h ) return;

	int x1 = x0 + xlen;

	if      ( x0 <  0            ) x0=0;
	else if ( x0 >= grfx->fbo->w ) x0=grfx->fbo->w-1;

	if ( x1 >= grfx->fbo->w ) x1=grfx->fbo->w-1;
	if ( x1 <= x0 ) return;

	for ( int x=x0; x<x1; ++x ) draw_at_unchecked( grfx, vec2i_(x,y) );
}
void draw_line_v( struct Graphics const* grfx, int x, int y0,unsigned ylen )
{
	if ( x<0 || x>=grfx->fbo->w ) return;

	int y1 = y0 + ylen;

	if      ( y0 <  0            ) y0=0;
	else if ( y0 >= grfx->fbo->h ) y0=grfx->fbo->h-1;

	if ( y1 >= grfx->fbo->h ) y1=grfx->fbo->h-1;
	if ( y1 <= y0 ) return;

	for ( int y=y0; y<y1; ++y ) draw_at_unchecked( grfx, vec2i_(x,y) );
}

void draw_rect( struct Graphics const* grfx, Rect rect, unsigned width )
{
	if      ( width == 0 )
	{
		FILL:

		if ( rect.x < 0 ) rect.x=0;
		if ( rect.y < 0 ) rect.y=0;
		if ( rect.x+rect.w >= grfx->fbo->w ) rect.w=(grfx->fbo->w-1)-rect.x;
		if ( rect.y+rect.h >= grfx->fbo->h ) rect.h=(grfx->fbo->h-1)-rect.y;

		for ( int j=rect.y; j<rect.y+rect.h; ++j )
		for ( int i=rect.x; i<rect.x+rect.w; ++i )
		{
			draw_at_unchecked( grfx, vec2i_(i,j) );
		}
	}
	else if ( width == 1 )
	{
		draw_line_h( grfx, rect.x,rect.w, rect.y          );
		draw_line_h( grfx, rect.x,rect.w, rect.y+rect.h-1 );
		draw_line_v( grfx, rect.x         , rect.y+1,rect.h-2 );
		draw_line_v( grfx, rect.x+rect.w-1, rect.y+1,rect.h-2 );
	}
	else if ( 2*width>=rect.w || 2*width>=rect.h ) goto FILL;
	else
	{
		//kassert( false, "test %d", 6 );
		draw_rect( grfx, Rect_( rect.x,rect.y             , rect.w,width ), 0 );
		draw_rect( grfx, Rect_( rect.x,rect.y+rect.h-width, rect.w,width ), 0 );
		draw_rect( grfx, Rect_( rect.x             ,rect.y+width, width,rect.h-2*width ), 0 );
		draw_rect( grfx, Rect_( rect.x+rect.w-width,rect.y+width, width,rect.h-2*width ), 0 );
	}
}

void draw_scroll_v( struct Image* img, int dy, sRGB_A empty )
{
	struct Graphics grfx = { .fbo=img };
	draw_set_color( &grfx, empty );

	size_t line_sz = 4 * img->w;
	if      ( dy > 0 )
	{
		uint32_t j = 0;
		for ( ; j+dy<img->h; ++j )
		{
			memcpy( img->data+j*line_sz, img->data+(j+dy)*line_sz, line_sz );
		}
		for ( ; j<img->h; ++j )
		{
			for ( uint32_t i=0; i<img->w; ++i )
			{
				draw_at_unchecked( &grfx, vec2i_(i,j) );
			}
		}
	}
	else if ( dy < 0 )
	{
		int j = (int)img->h - 1;
		for ( ; j+dy>=0; --j )
		{
			memcpy( img->data+j*line_sz, img->data+(j+dy)*line_sz, line_sz );
		}
		for ( ; j>=0; --j )
		{
			for ( uint32_t i=0; i<img->w; ++i )
			{
				draw_at_unchecked( &grfx, vec2i_(i,j) );
			}
		}
	}
}

void draw_blit( struct Graphics const* grfx, struct Image const* src_img, vec2i pos )
{
	struct Image* dst_img = grfx->fbo;

	Rect inter;
	bool ret = rect_intersection(
		Rect_( pos.x,pos.y, src_img->w,src_img->h ),
		Rect_( 0,0, dst_img->w,dst_img->h ),
		&inter
	);
	if ( !ret ) return;

	for ( uint32_t j=(uint32_t)inter.y; j<(uint32_t)inter.y+inter.h; ++j )
	for ( uint32_t i=(uint32_t)inter.x; i<(uint32_t)inter.x+inter.w; ++i )
	{
		uint32_t color = draw_get_pixel_at( src_img, vec2i_(i-pos.x,j-pos.y) );
		draw_set_pixel_at( dst_img, vec2i_(i,j), color );
	}
}
