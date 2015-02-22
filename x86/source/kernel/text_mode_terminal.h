#pragma once
#include "../includes.h"


namespace MOSS { namespace Terminal {


//NOTE: All numbers and coordinates start at 0!

class TextModeTerminal {
	private:
		//char data[VGA_HEIGHT][VGA_WIDTH];

		int _x;
		int _y;
		uint8_t color;
		uint16_t* buffer;

	public:
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
		TextModeTerminal(void);
		~TextModeTerminal(void);

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

		void write(const char* format, ...);
};


}}