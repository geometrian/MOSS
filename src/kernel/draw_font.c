#include "draw.h"



/*
PSF Fonts:
	https://en.wikipedia.org/wiki/PC_Screen_Font
	https://wiki.osdev.org/PC_Screen_Font
*/



struct _PSF1_FontHeader
{
	uint8_t magic[2]; // 0x36, 0x04
	uint8_t mode;
	uint8_t glyph_sz;
};
static_assert( sizeof(struct _PSF1_FontHeader) == 4 );

struct _PSF2_FontHeader
{
	uint8_t magic[4]; // 0x72, 0xb5, 0x4a, 0x86
	uint32_t version; // 0
	uint32_t header_sz; // probably 32
	uint32_t flags;
	uint32_t glyph_count;
	uint32_t glyph_sz;
	uint32_t glyph_h; // yes, height first
	uint32_t glyph_w;
};
static_assert( sizeof(struct _PSF2_FontHeader) == 32 );

bool draw_load_font_ptr( void const* ptr, struct BitmapFont* font )
{
	union
	{
		uint8_t  const* u8 ;
		uint16_t const* u16;
		uint32_t const* u32;
		uint64_t const* u64;
		void     const* v;
		struct _PSF1_FontHeader const* psf1;
		struct _PSF2_FontHeader const* psf2;
	}
	u;
	u.v = ptr;

	if      ( u.u16[0] == 0x0436 )
	{
		// PSF 1 Font

		font->glyph_w = 8;
		font->glyph_h = u.psf1->glyph_sz; // same as height, since width is always 8

		font->glyph_scanline_sz = ( font->glyph_w + 7 ) / 8;
		font->glyph_sz = font->glyph_h * font->glyph_scanline_sz;

		font->glyph_count = ( u.psf1->mode & 0x01 ) ? 512 : 256;
		// TODO: unicode translation table in flags too . . .

		font->glyphs_data = u.u8 + sizeof(struct _PSF1_FontHeader);
	}
	else if ( u.u32[0] == 0x864ab572u )
	{
		// PSF 2 Font

		if ( u.psf2->glyph_w>255 || u.psf2->glyph_h>255 ) return false;
		font->glyph_w = (uint8_t)u.psf2->glyph_w;
		font->glyph_h = (uint8_t)u.psf2->glyph_h;

		font->glyph_scanline_sz = ( font->glyph_w + 7 ) / 8;
		font->glyph_sz = font->glyph_h * font->glyph_scanline_sz;

		font->glyph_count = u.psf2->glyph_count;

		// TODO: unicode translation table in flags . . .

		font->glyphs_data = u.u8 + u.psf2->header_sz;
	}
	else return false;

	return true;
}



static int _draw_printf_helper( int ch, void* grfx_ptr )
{
	struct Graphics* grfx = (struct Graphics*)grfx_ptr;
	draw_render_ch( grfx, (char)ch );
	return ch;
}

void draw_render_ch     ( struct Graphics* grfx, char        ch                    )
{
	if ( ch == '\n' )
	{
		grfx->text_pos.x = grfx->text_x_wrap;
		grfx->text_pos.y += grfx->font->glyph_h;
		return;
	}

	uint8_t const* glyph_data = (uint8_t const*)( grfx->font->glyphs_data );
	if ( ch < grfx->font->glyph_count ) glyph_data+=ch*grfx->font->glyph_sz;

	for ( uint8_t j=0; j<grfx->font->glyph_h; ++j )
	for ( uint8_t i=0; i<grfx->font->glyph_w; ++i )
	{
		uint8_t bit = glyph_data[ j*grfx->font->glyph_scanline_sz + i/8 ] & ( 1 << (7-i%8) );
		if ( bit )
		{
			// TODO: `draw_at_unchecked(⋯)`
			draw_at( grfx, vec2i_(grfx->text_pos.x+i,grfx->text_pos.y+j) );
		}
	}
	#if 0
	draw_at( grfx, vec2i_(grfx->text_pos.x                    ,grfx->text_pos.y                    ) );
	draw_at( grfx, vec2i_(grfx->text_pos.x+grfx->font->glyph_w,grfx->text_pos.y                    ) );
	draw_at( grfx, vec2i_(grfx->text_pos.x+grfx->font->glyph_w,grfx->text_pos.y+grfx->font->glyph_h) );
	draw_at( grfx, vec2i_(grfx->text_pos.x                    ,grfx->text_pos.y+grfx->font->glyph_h) );
	#endif

	grfx->text_pos.x += grfx->font->glyph_w;
}
void draw_render_str    ( struct Graphics* grfx, char const* str                   )
{
	char ch;
	while ( (ch=*str) )
	{
		draw_render_ch( grfx, ch );
		// advance already in `draw_render_ch(⋯)`

		++str;
	}
}
void draw_render_printf ( struct Graphics* grfx, char const* fmt_cstr,...          )
{
	va_list args;
	va_start( args, fmt_cstr );
	draw_render_vprintf( grfx, fmt_cstr,args );
	va_end( args );
}
void draw_render_vprintf( struct Graphics* grfx, char const* fmt_cstr,va_list args )
{
	FILE file_fn = __moss_file_fn( &_draw_printf_helper, grfx );
	vfprintf( &file_fn, fmt_cstr, args );
}

void draw_render_stacktrace( struct Graphics* grfx )
{
	FILE file_fn = __moss_file_fn( &_draw_printf_helper, grfx );
	__moss_fput_stack( &file_fn );
}
