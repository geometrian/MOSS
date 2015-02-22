#include "text_mode_terminal.h"

#include "../mossc/cstdio"

#include "io/io.h"


namespace MOSS { namespace Terminal {


#define TEXTMODE_WIDTH 80
#define TEXTMODE_HEIGHT 25

#define EMPTY_CH ' '
#define EMPTY_FG COLOR_LIGHT_GREY
#define EMPTY_BG COLOR_BLACK

#define DEFAULT_FG COLOR_DARK_GREY
#define DEFAULT_BG COLOR_BLACK


TextModeTerminal::TextModeTerminal(void) {
	_x = 0;
	_y = 0;
	buffer = reinterpret_cast<uint16_t*>(0xB8000);

	set_color(EMPTY_FG,EMPTY_BG);
	for (int y=0; y<TEXTMODE_HEIGHT; ++y) {
		for (int x=0; x<TEXTMODE_WIDTH; ++x) {
			write(EMPTY_CH, x,y);
		}
	}

	set_color(DEFAULT_FG,DEFAULT_BG);
}
TextModeTerminal::~TextModeTerminal(void) {}

void TextModeTerminal::scroll(int lines) {
	for (int i=0;i<lines;++i) {
		for (int y=0; y<TEXTMODE_HEIGHT-1; ++y) {
			for (int x=0; x<TEXTMODE_WIDTH; ++x) {
				int index0 =  y   *TEXTMODE_WIDTH + x;
				int index1 = (y+1)*TEXTMODE_WIDTH + x;
				buffer[index0] = buffer[index1];
			}
		}
	}
}

void TextModeTerminal::fill_line(int number, char with) {
	for (int x=0; x<TEXTMODE_WIDTH; ++x) {
		write(with, x,number);
	}
}

void TextModeTerminal::next_line(void) {
	_x = 0;
	if (++_y == TEXTMODE_HEIGHT) {
		scroll(1);
		--_y;

		uint8_t temp_color = color;
		set_color(EMPTY_FG,EMPTY_BG);
		fill_line(TEXTMODE_HEIGHT-1,EMPTY_CH);
		color = temp_color;
	}
}

void TextModeTerminal::set_color_text(enum COLORS color_text) {
	color &= 0x00F0u;
	color |= color_text;
}
void TextModeTerminal::set_color_background(enum COLORS color_background) {
	color &= 0x000Fu;
	color |= color_background<<4;
}
void TextModeTerminal::set_color(enum COLORS color_text, enum COLORS color_background) {
	color = (color_background<<4) | color_text;
}

void TextModeTerminal::write(char c, int x,int y) {
	int index = y*TEXTMODE_WIDTH + x;

	uint16_t c16 = c;
	uint16_t color16 = color;

	buffer[index] = c16 | (color16 << 8);
}
void TextModeTerminal::write(char c) {
	#ifdef MOSS_DEBUG
	//Bochs debug: TODO: it doesn't do anything.
	//IO::send(0xE9,static_cast<uint8_t>(c));
	#endif

	if (c=='\r') {
		_x = 0;
	} else if (c=='\n') {
		next_line();
	} else {
		write(c, _x,_y);

		if (++_x == TEXTMODE_WIDTH) {
			next_line();
		}
	}
}
void TextModeTerminal::write(char const* format) {
	size_t i = 0;
	LOOP:
		char c = format[i];
		if (c!='\0') {
			write(c);
			++i;
			goto LOOP;
		}
}
void TextModeTerminal::write(char const* format, va_list args) {
	char buffer[256]; //TODO: use vsnprintf or something!

	MOSSC::vsprintf(buffer,format,args);
	size_t i = 0;
	LOOP:
		char c = buffer[i];
		if (c!='\0') {
			write(c);
			++i;
			goto LOOP;
		}
}


}}