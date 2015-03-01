#include "terminal.h"

#include "../../../mossc/cstdio"

#include "../../io/io.h"


namespace MOSS { namespace Graphics { namespace VGA {


#define EMPTY_CH ' '
#define EMPTY_FG COLOR_LIGHT_GREY
#define EMPTY_BG COLOR_BLACK

#define DEFAULT_FG COLOR_DARK_GREY
#define DEFAULT_BG COLOR_BLACK


Terminal::Terminal(void) {
	_x = 0;
	_y = 0;
	_buffer = reinterpret_cast<uint16_t*>(0xB8000);

	//interface.crtc.set_mode(CathodeRayTubeController::Mode::text80x25);

	//while(1);

	set_color(EMPTY_FG,EMPTY_BG);
	for (int y=0; y<interface.crtc.rows; ++y) {
		for (int x=0; x<interface.crtc.cols; ++x) {
			write(EMPTY_CH, x,y);
		}
	}

	set_color(DEFAULT_FG,DEFAULT_BG);
}
Terminal::~Terminal(void) {}

void Terminal::scroll(int lines) {
	for (int i=0;i<lines;++i) {
		for (int y=0; y<interface.crtc.rows-1; ++y) {
			for (int x=0; x<interface.crtc.cols; ++x) {
				int index0 =  y   *interface.crtc.cols + x;
				int index1 = (y+1)*interface.crtc.cols + x;
				_buffer[index0] = _buffer[index1];
			}
		}
	}
}

void Terminal::fill_line(int number, char with) {
	for (int x=0; x<interface.crtc.cols; ++x) {
		write(with, x,number);
	}
}

void Terminal::next_line(void) {
	_x = 0;
	if (++_y == interface.crtc.rows) {
		scroll(1);
		--_y;

		uint8_t temp_color = _color;
		set_color(EMPTY_FG,EMPTY_BG);
		fill_line(interface.crtc.rows-1,EMPTY_CH);
		_color = temp_color;
	}
}

void Terminal::set_color_text(enum COLORS color_text) {
	_color &= 0x00F0u;
	_color |= color_text;
}
void Terminal::set_color_background(enum COLORS color_background) {
	_color &= 0x000Fu;
	_color |= color_background<<4;
}
void Terminal::set_color(enum COLORS color_text, enum COLORS color_background) {
	_color = (color_background<<4) | color_text;
}

void Terminal::write(char c, int x,int y) {
	int index = y*interface.crtc.cols + x;

	uint16_t c16 = c;
	uint16_t color16 = _color;

	_buffer[index] = c16 | (color16 << 8);
}
void Terminal::write(char c) {
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

		if (++_x == interface.crtc.cols) {
			next_line();
		}
	}
}
void Terminal::write(char const* format) {
	size_t i = 0;
	LOOP:
		char c = format[i];
		if (c!='\0') {
			write(c);
			++i;
			goto LOOP;
		}
}
void Terminal::write(char const* format, va_list args) {
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


}}}
