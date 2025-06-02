#include "text_ui.h"



void textui_init( struct TextUI* textui )
{
	textui->matr.size.x = TEXTUI_W;
	textui->matr.size.y = TEXTUI_H;

	textui->pos = vec2u_( 0, 0 );
	textui->color = sRGB_( 255, 255, 255 );

	textui_fill( textui, ' ' );
}

void textui_fill( struct TextUI* textui, char ch )
{
	struct TextUI_Char fill = { .ch=ch, .color=textui->color };

	for ( size_t j=0; j<textui->matr.size.y; ++j )
	for ( size_t i=0; i<textui->matr.size.x; ++i )
	{
		textui->matr.buffer[ j*textui->matr.size.x + i ] = fill;
	}
}

void textui_newline( struct TextUI* textui )
{
	textui->pos.x = 0;
	if ( ++textui->pos.y == textui->matr.size.y )
	{
		textui_scroll_down( textui, 1 );
	}
}

void textui_scroll_down( struct TextUI* textui, unsigned count )
{
	struct TextUI_CharMatrix* matr = &textui->matr;

	size_t row_size = textui->matr.size.x * sizeof(struct TextUI_Char);
	struct TextUI_Char empty = { .ch=' ', .color=textui->color };

	while ( count > 0 )
	{
		for ( size_t j=0; j<matr->size.y-1; ++j ) memcpy(
			(uint8_t*)matr->buffer +  j   *row_size,
			(uint8_t*)matr->buffer + (j+1)*row_size,
			row_size
		);
		for ( size_t i=0; i<matr->size.x; ++i ) matr->buffer[ (matr->size.y-1)*matr->w + i ]=empty;

		--textui->pos.y;
		if ( textui->pos.y < 0 ) textui->pos.y=0;

		--count;
	}
}

static int _textui_write_ch( int ch, void*/* (struct TextUI*) */ textui_ptr )
{
	struct TextUI* textui = (struct TextUI*)textui_ptr;
	if ( ch == '\n' )
	{
		textui_newline( textui );
	}
	else
	{
		if ( textui->pos.x == textui->matr.size.x-1 ) textui_newline( textui );

		textui->matr.buffer[ textui->pos.y*textui->matr.size.x + textui->pos.x ] =
			(struct TextUI_Char)
			{
				.ch = ch,
				.color = textui->color
			}
		;
		++textui->pos.x;
	}
	return ch;
}
int textui_putc( struct TextUI* textui, char ch )
{
	FILE file_fn = __moss_file_fn( &_textui_write_ch, textui );
	return fputc( ch, &file_fn );
}
int textui_puts( struct TextUI* textui, char const* str )
{
	FILE file_fn = __moss_file_fn( &_textui_write_ch, textui );
	return fputs( str, &file_fn );
}
__attribute__(( format(printf,2,3) ))
int textui_printf ( struct TextUI* textui, char const* fmt_cstr,...          )
{
	va_list args;
	va_start( args, fmt_cstr );
	int ret = textui_vprintf( textui, fmt_cstr,args );
	va_end( args );

	return ret;
}
__attribute__(( format(printf,2,0) ))
int textui_vprintf( struct TextUI* textui, char const* fmt_cstr,va_list args )
{
	FILE file_fn = __moss_file_fn( &_textui_write_ch, textui );
	return vfprintf( &file_fn, fmt_cstr,args );
}

void textui_draw( struct TextUI const* textui, struct Graphics* grfx )
{
	struct BitmapFont const* font = grfx->font;

	draw_rect_color(
		grfx,
		sRGB_A_( 0,0,0, 255 ),
		Rect_( 0,0, textui->matr.size.x*font->glyph_w, textui->matr.size.y*font->glyph_h ),
		0
	);
	#if 1
	draw_rect_color(
		grfx,
		sRGB_A_( textui->color.r,textui->color.g,textui->color.b, 255 ),
		Rect_( 0,0, textui->matr.size.x*font->glyph_w, textui->matr.size.y*font->glyph_h ),
		1
	);
	#endif

	for ( size_t j=0; j<textui->matr.size.y; ++j )
	for ( size_t i=0; i<textui->matr.size.x; ++i )
	{
		struct TextUI_Char chr = textui->matr.buffer[ j*textui->matr.size.x + i ];

		grfx->text_pos = vec2i_( i*font->glyph_w, j*font->glyph_h );

		draw_set_color( grfx, sRGB_A_(chr.color.r,chr.color.g,chr.color.b,255) );
		draw_render_ch( grfx, chr.ch );
	}
}
