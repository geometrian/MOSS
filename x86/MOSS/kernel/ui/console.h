#pragma once

#include "keyboard/keyboard.h"

namespace CONSOLE {

#define NUM_LINES 3
//#define LINE_LENGTH 56 /*good for font 1*/
#define LINE_LENGTH 38 /*good for font 2*/

class Cell {
	private:
		bool dirty;
		char value;
	public:
		Cell(void) : dirty(false),value(' ') {}

		Cell& operator=(const Cell& other) {
			dirty = true;
			value = other.value;
			return *this;
		}

		void set_value(char new_value);
		void draw(int x, int y);
};

class Console {
	private:
		int line_number_index;

		int line_position_index;
		Cell lines[NUM_LINES][LINE_LENGTH];
	public:
		Console(void) {
			line_number_index = 0;
			setup_current_line();
		}
	private:
		void setup_current_line(void) {
			lines[line_number_index][0].set_value('>');
			lines[line_number_index][1].set_value('>');
			lines[line_number_index][2].set_value('>');
			lines[line_number_index][3].set_value(' ');
			for (int i=4;i<LINE_LENGTH;++i) lines[line_number_index][i].set_value(' '); //erase the rest of the line
			line_position_index = 4;
		}
	public:
		void update(void) {
			if ( KEYBOARD::inportb(0x64)&0x01 ) { //0x64 is for reading the 8042 status register
				lines[line_number_index][line_position_index].set_value(' '); //Erase cursor, if it happened to be blinking on

				unsigned char key = KEYBOARD::convert(KEYBOARD::inportb(0x60));
				if (key != '\0') {
					if (key=='\n') {
						if (line_number_index==NUM_LINES-1) {
							for (int i=1;i<NUM_LINES;++i) {
								for (int j=0;j<LINE_LENGTH;++j) {
									lines[i-1][j] = lines[i][j];
								}
							}
						} else {
							++line_number_index;
						}
						setup_current_line();
					} else if (key=='\b') {
						if (line_position_index>4) {
							--line_position_index;
						}
					} else if (line_position_index<LINE_LENGTH-1) { //-1 because we need space for the cursor to blink
						lines[line_number_index][line_position_index++].set_value(key);
					}
				}
				//CONSOLE::Console::draw(0,22,(int)(key));
			}

			static signed char cursor = -128;
			++cursor;
			lines[line_number_index][line_position_index].set_value((cursor>0)?'#':' ');

			for (int y=0;y<NUM_LINES;++y) {
				for (int x=0;x<LINE_LENGTH;++x) {
					lines[y][x].draw(x,y+1);
				}
			}

			/*if ( KEYBOARD::inportb(0x64)&0x01 ) { // 0x64 is for reading the 8042 status register
				CONSOLE::Console::draw(5,8,"HOORAY!");
				const char out[4] = {'"',KEYBOARD::convert(KEYBOARD::inportb(0x60)),'"','\0'};
				CONSOLE::Console::draw(5,9,out);
			}*/

			//Delay for cursor blinking's benefit
			for (int i=0;i<100;++i) {
				for (int j=0;j<100;++j) {
					for (int k=0;k<2;++k) {}
				}
			}

			CONSOLE::Console::draw(0,23,"Line:"); CONSOLE::Console::draw(6,23,line_number_index+1);
			CONSOLE::Console::draw(0,24,"Col:" ); CONSOLE::Console::draw(6,24,line_position_index+1);
		}

		static void draw(int x, int y, char c);
		static void draw(int x, int y, const char* str);

		static void draw(int x, int y, unsigned int num, int base=10);
		static void draw(int x, int y, int num, int base=10);

		/*void print(const char* format, const char* args) {
			short making = 0x0000;
			char c;
			LOOP:
				c = *format;
				if (c=='\0') return;
				else if (c=='\n') { ++cursor_y; }
				else if (c=='\r') { cursor_x = 0; }
				else {
					if (c=='%') {
						if (making!=0x0000) goto MAKE;
						making = 0x0000;
					}
				}
				goto LOOP;

				MAKE:
			
					goto LOOP;
		}*/
};

#undef LINE_LENGTH
#undef NUM_LINES
/*int add_all(int count, ...) {
	va_list ap;
	int i, sum;

	va_start(ap, count);

	sum = 0;
	for (i = 0; i < count; i++)
		sum += va_arg(ap,int);

	va_end (ap);
	return sum;
}*/

}