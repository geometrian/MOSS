#include "terminal.h"

#include "../../../mossc/cstdio"
#include "../../../mossc/cstring"

#include "../../io/io.h"

#include "../font.h"


namespace MOSS { namespace Graphics { namespace VGA {


Terminal::Terminal(void) {
	_x = 0;
	_y = 0;
	_buffer = reinterpret_cast<uint16_t*>(0xB8000);

	//This seems to be the default
	color_text = COLOR_LIGHT_GREY;
	color_bg   = COLOR_BLACK;

	#if 1
		interface.set_use_font(Graphics::Font::font8x8);
		interface.crtc.set_mode(Graphics::VGA::CathodeRayTubeController::Mode::text128x80);
	#else
		interface.set_use_font(Graphics::Font::font8x16);
		interface.crtc.set_mode(Graphics::VGA::CathodeRayTubeController::Mode::text128x48);
	#endif

	for (int y=0; y<interface.crtc.rows; ++y) {
		for (int x=0; x<interface.crtc.cols; ++x) {
			write(' ', x,y);
		}
	}

	/*//char const*  str80 = "<-|(((Hello_world; how_are_you?__My_name_is_Ian;_good_to_finally_meet_you!)))|->";
	//char const*  str90 = "<-|((Hello_world; how_are_you?__My_name_is_Ian;_it's_good_to_finally_get_to_meet_you!))|->";
	//char const* str132 = "<-|(((Hello_world; how_are_you?__My_name_is_Ian!__Wow this is exciting.__I'm_so_happy.__It_is_good_to_finally_get_to_meet_you!)))|->";
	char const* str132 = "*1 5*  10*  15*  20*  25*  30*  35*  40*  45*  50*  55*  60*  65*  70*  75*  80*  85*  90*  95* 100* 105* 110* 115* 120* 125* 130* |";
	for (int j=0;j<1;++j) {
		set_color_text(Graphics::VGA::Terminal::COLOR_RED  ); kernel->write("%s",str132);
		set_color_text(Graphics::VGA::Terminal::COLOR_GREEN); kernel->write("%s",str132);
		set_color_text(Graphics::VGA::Terminal::COLOR_BLUE ); kernel->write("%s",str132);
	}
	set_color_text(Graphics::VGA::Terminal::COLOR_RED  );
	set_pos(0,0);
	for (int i=0;i<80;++i) {
		kernel->write("%d\n",i);
	}
	while (1);*/
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
		/*scroll(1);
		--_y;

		uint8_t temp_color = _color;
		set_color(EMPTY_FG,EMPTY_BG);
		fill_line(interface.crtc.rows-1,EMPTY_CH);
		_color = temp_color;*/
	}
}

void Terminal::set_color_text(enum Color color_text) {
	if (this->color_text!=color_text) {
		_color &= 0x00F0u;
		_color |= color_text;

		this->color_text = color_text;
	}
}
void Terminal::set_color_background(enum Color color_bg) {
	if (this->color_bg!=color_bg) {
		_color &= 0x000Fu;
		_color |= color_bg<<4;

		this->color_bg = color_bg;
	}
}
void Terminal::set_color(enum Color color_text, enum Color color_bg) {
	set_color_text(color_text);
	set_color_background(color_bg);
	//_color = (color_bg<<4) | color_text;
}

void Terminal::set_pos(int x, int y) {
	_x = x;
	_y = y;
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
			//next_line();
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
	#ifdef MOSS_DEBUG
	int len = MOSSC::strlen(format);
	assert_term(len<1024,"Format string too long (%d)!  Not implemented!",len);
	#endif
	char buffer[1024]; //TODO: use vsnprintf or something!

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
