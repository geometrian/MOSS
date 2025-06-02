#pragma once

//#include "stdafx.h"

#include "color.h"



typedef enum
{
	COLOR_NONE = 0, // good value for memset 0 to create
	COLOR_A_sRGB,
	COLOR_sRGB_A,
	COLOR_A_sBGR,
	COLOR_sBGR_A
}
COLOR;

struct Image
{
	COLOR color_type;
	union
	{
		vec2u res;
		struct { uint32_t w, h; };
	};
	uint32_t scanline_sz;
	void* data;
	/*union
	{
		A_sRGB* a_srgb;
	}
	data;*/
};



struct BitmapFont
{
	uint8_t glyph_w;
	uint8_t glyph_h;

	uint8_t glyph_scanline_sz;
	uint8_t glyph_sz;

	uint32_t glyph_count;

	void const* glyphs_data;
};



struct Graphics
{
	struct Image* fbo;

	uint32_t color_native;

	struct BitmapFont const* font;
	vec2i text_pos;
	int   text_x_wrap;
};



void draw_img_init( struct Image* img, COLOR color_type, vec2u res, void* data );

void draw_grfx_init( struct Graphics* grfx, struct Image* fbo, struct BitmapFont const* font );

void draw_set_color( struct Graphics* grfx, sRGB_A color );

MOSS_ND_INLINE size_t draw_get_offset_at( struct Image const* img, vec2i valid_pos )
{
	kassert(
		valid_pos.x>=0 && valid_pos.y>=0 &&
		valid_pos.x<img->w && valid_pos.y<img->h,
		"Coordinate <%d,%d> outside of %ux%u image!",
		valid_pos.x, valid_pos.y,
		img->w, img->h
	);
	return valid_pos.y*img->scanline_sz + 4*valid_pos.x;
}
MOSS_ND_INLINE uint32_t draw_get_pixel_at( struct Image const* img, vec2i valid_pos )
{
	return *(uint32_t const*)( (uint8_t const*)img->data + draw_get_offset_at(img,valid_pos) );
}
MOSS_INLINE void draw_set_pixel_at( struct Image const* img, vec2i valid_pos, uint32_t color_native )
{
	*(uint32_t*)( (uint8_t*)img->data + draw_get_offset_at(img,valid_pos) ) = color_native;
}

// (Sets `grfx`'s color, )checks `pos` and, if valid, draws a point there.  Returns whether drew.
bool             draw_at          ( struct Graphics const* grfx,               vec2i pos       );
MOSS_INLINE bool draw_at_color    ( struct Graphics      * grfx, sRGB_A color, vec2i pos       )
{
	draw_set_color( grfx, color );
	return draw_at( grfx, pos );
}
// Draws pixel at (valid!) position `valid_pos` using `grfx`'s current color
MOSS_INLINE void draw_at_unchecked( struct Graphics const* grfx,               vec2i valid_pos )
{
	draw_set_pixel_at( grfx->fbo, valid_pos, grfx->color_native );
}

void             draw_fill      ( struct Graphics const* grfx               );
MOSS_INLINE void draw_fill_color( struct Graphics      * grfx, sRGB_A color )
{
	draw_set_color( grfx, color );
	return draw_fill( grfx );
}

//Draws line at horizontal locations [`x0`,`x0+xlen`) and vertical position `y`.
void             draw_line_h      ( struct Graphics const* grfx,               int x0,unsigned xlen, int y );
MOSS_INLINE void draw_line_h_color( struct Graphics      * grfx, sRGB_A color, int x0,unsigned xlen, int y )
{
	draw_set_color( grfx, color );
	return draw_line_h( grfx, x0,xlen, y );
}
//Draws line at horizontal position `x` and vertical locations [`y0`,`y0+ylen`).
void             draw_line_v      ( struct Graphics const* grfx,               int x, int y0,unsigned ylen );
MOSS_INLINE void draw_line_v_color( struct Graphics      * grfx, sRGB_A color, int x, int y0,unsigned ylen )
{
	draw_set_color( grfx, color );
	return draw_line_v( grfx, x, y0,ylen );
}

void             draw_rect      ( struct Graphics const* grfx,               Rect rect, unsigned width );
MOSS_INLINE void draw_rect_color( struct Graphics      * grfx, sRGB_A color, Rect rect, unsigned width )
{
	draw_set_color( grfx, color );
	return draw_rect( grfx, rect, width );
}

void draw_scroll_v( struct Image* img, int dy, sRGB_A empty );

void draw_blit( struct Graphics const* grfx, struct Image const* src_img, vec2i pos );



bool draw_load_font_ptr( void const* ptr, struct BitmapFont* font );

void draw_render_ch     ( struct Graphics* grfx, char        ch                    );
void draw_render_str    ( struct Graphics* grfx, char const* str                   );
void draw_render_printf ( struct Graphics* grfx, char const* fmt_cstr,...          );
void draw_render_vprintf( struct Graphics* grfx, char const* fmt_cstr,va_list args );

void draw_render_stacktrace( struct Graphics* grfx );
