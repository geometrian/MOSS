#pragma once

#include "../../../includes.h"

#include "components.h"


namespace MOSS { namespace Graphics { namespace VGA {


//Note: all numbers and coordinates start at zero.

class Terminal final {
	private:
		//char data[VGA_HEIGHT][VGA_WIDTH];

		int _x;
		int _y;
		uint8_t _color;
		uint16_t volatile* _buffer;

	public:
		Interface interface;

		enum COLORS {
			COLOR_BLACK         =  0,
			COLOR_BLUE          =  1,
			COLOR_GREEN         =  2,
			COLOR_CYAN          =  3,
			COLOR_RED           =  4,
			COLOR_MAGENTA       =  5,
			COLOR_BROWN         =  6,
			COLOR_LIGHT_GREY    =  7,
			COLOR_DARK_GREY     =  8,
			COLOR_LIGHT_BLUE    =  9,
			COLOR_LIGHT_GREEN   = 10,
			COLOR_LIGHT_CYAN    = 11,
			COLOR_LIGHT_RED     = 12,
			COLOR_LIGHT_MAGENTA = 13,
			COLOR_LIGHT_BROWN   = 14,
			COLOR_WHITE         = 15,
		};

	public:
		Terminal(void);
		~Terminal(void);

		void scroll(int lines);

		void fill_line(int number, char with);

		void next_line(void);

		void set_color_text(enum COLORS color_text);
		void set_color_background(enum COLORS color_background);
		void set_color(enum COLORS color_text, enum COLORS color_background);

		void write(char c, int x,int y);
		void write(char c);
		/*void write(  int data);
		void write( bool data);
		void write(void* data);*/
		void write(char const* format);
		void write(char const* format, va_list args);
};


}}}
