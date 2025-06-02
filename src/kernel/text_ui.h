#pragma once

//#include "stdafx.h"

#include "draw.h"



#if 1 // 1024x768 8x16 font
	#define TEXTUI_W (128-1)
	#define TEXTUI_H ( 48-1)
#else //1920x1080 5x9 font
	#define TEXTUI_W (384-1)
	#define TEXTUI_H (120-1)
#endif

struct TextUI_Char
{
	char ch;
	sRGB color;
};
struct TextUI_CharMatrix
{
	union
	{
		vec2u size;
		struct { uint32_t w, h; };
	};
	struct TextUI_Char buffer[ TEXTUI_H * TEXTUI_W ];
};

struct TextUI
{
	struct TextUI_CharMatrix matr;

	vec2u pos;
	sRGB color; // user can just set
};



void textui_init( struct TextUI* textui );

void textui_fill( struct TextUI* textui, char ch );

void textui_newline( struct TextUI* textui );

void textui_scroll_down( struct TextUI* textui, unsigned count );

int textui_putc( struct TextUI* textui, char ch );
int textui_puts( struct TextUI* textui, char const* str );
__attribute__(( format(printf,2,3) ))
int textui_printf ( struct TextUI* textui, char const* fmt_cstr,...          );
__attribute__(( format(printf,2,0) ))
int textui_vprintf( struct TextUI* textui, char const* fmt_cstr,va_list args );

void textui_draw( struct TextUI const* textui, struct Graphics* grfx );
