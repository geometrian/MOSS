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

		enum Color {
			//Each enum representing a color has a bitwise meaning:
			//	0 b 0 0 0 0 <intense> <red> <green> <blue>

			COLOR_BLACK       = 0x00,

			COLOR_BLUE        = 0x01,
			COLOR_GREEN       = 0x02,
			COLOR_CYAN        = 0x03,
			COLOR_RED         = 0x04,
			COLOR_PURPLE      = 0x05,
			COLOR_BROWN       = 0x06,

			COLOR_LIGHT_GREY  = 0x07,
			COLOR_DARK_GREY   = 0x08,

			COLOR_LIGHT_BLUE  = 0x09,
			COLOR_LIGHT_GREEN = 0x0A,
			COLOR_LIGHT_CYAN  = 0x0B,
			COLOR_LIGHT_RED   = 0x0C,
			COLOR_MAGENTA     = 0x0D,
			COLOR_YELLOW      = 0x0E,

			COLOR_WHITE       = 0x0F
		};

		Color color_text;
		Color color_bg;

	public:
		Terminal(void);
		~Terminal(void);

		void scroll(int lines);

		void fill_line(int line_index, char with);

		void next_line(void);

		void set_color_text(enum Color color_text);
		void set_color_background(enum Color color_bg);
		void set_color(enum Color color_text, enum Color color_bg);

		void set_pos(int x, int y);

		void write(char c, int x,int y);
		void write(char c);
		/*void write(  int data);
		void write( bool data);
		void write(void* data);*/
		void write(char const* format);
		void write(char const* format, va_list args);
};


}}}
