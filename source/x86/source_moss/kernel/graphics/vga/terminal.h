#pragma once

#include "../../../includes.h"

#include "components.h"


namespace MOSS { namespace Graphics { namespace VGA {


class Device;


//Note: all numbers and coordinates start at zero.

class Terminal final {
	private:
		Device*const _device;

		size_t _x;
		size_t _y;
		uint8_t _color;
		uint16_t volatile*const _buffer;

	public:
		enum class Color : uint8_t {
			//Each enum representing a color has a bitwise meaning:
			//	0 b 0 0 0 0 <intense> <red> <green> <blue>
			//	                           I RGB

			BLACK       = 0x00, //0b 0 000

			BLUE        = 0x01, //0b 0 001
			GREEN       = 0x02, //0b 0 010
			CYAN        = 0x03, //0b 0 011
			RED         = 0x04, //0b 0 100
			PURPLE      = 0x05, //0b 0 101
			BROWN       = 0x06, //0b 0 110

			LIGHT_GREY  = 0x07, //0b 0 111
			DARK_GREY   = 0x08, //0b 1 000

			LIGHT_BLUE  = 0x09, //0b 1 001
			LIGHT_GREEN = 0x0A, //0b 1 010
			LIGHT_CYAN  = 0x0B, //0b 1 011
			LIGHT_RED   = 0x0C, //0b 1 100
			MAGENTA     = 0x0D, //0b 1 101
			YELLOW      = 0x0E, //0b 1 110

			WHITE       = 0x0F  //0b 1 111
		};

		Color color_text;
		Color color_bg;

	public:
		explicit Terminal(Device* device);
		inline ~Terminal(void) = default;

		void clear(void);

		void scroll(size_t lines);

		void fill_line(size_t line_index, char with);

		void next_line(void);

		void set_color_text(Color color_text);
		void set_color_background(Color color_bg);
		void set_color(Color color_text, Color color_bg);

		inline void set_pos_x(size_t x) { _x=x; }
		inline void set_pos_y(size_t y) { _y=y; }
		inline void set_pos(size_t x, size_t y) { _x=x; _y=y; }

		void write(char c, size_t x,size_t y);
		void write(char c);
		/*void write(  int data);
		void write( bool data);
		void write(void* data);*/
		//Must be in header for some reason.
		inline void write(char const* format, ...) {
			va_list args; va_start(args,format);
			write(format,args);
			va_end(args);
		}
		void write(char const* format, va_list args);

		void write_test_pattern_res(void);
		void write_test_pattern_lin(void);
};


}}}
