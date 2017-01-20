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
			//	                           I RGB

			COLOR_BLACK       = 0x00, //0b 0 000

			COLOR_BLUE        = 0x01, //0b 0 001
			COLOR_GREEN       = 0x02, //0b 0 010
			COLOR_CYAN        = 0x03, //0b 0 011
			COLOR_RED         = 0x04, //0b 0 100
			COLOR_PURPLE      = 0x05, //0b 0 101
			COLOR_BROWN       = 0x06, //0b 0 110

			COLOR_LIGHT_GREY  = 0x07, //0b 0 111
			COLOR_DARK_GREY   = 0x08, //0b 1 000

			COLOR_LIGHT_BLUE  = 0x09, //0b 1 001
			COLOR_LIGHT_GREEN = 0x0A, //0b 1 010
			COLOR_LIGHT_CYAN  = 0x0B, //0b 1 011
			COLOR_LIGHT_RED   = 0x0C, //0b 1 100
			COLOR_MAGENTA     = 0x0D, //0b 1 101
			COLOR_YELLOW      = 0x0E, //0b 1 110

			COLOR_WHITE       = 0x0F  //0b 1 111
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
